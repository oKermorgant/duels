#ifndef DUELS_SERVER_H
#define DUELS_SERVER_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>

#include <duels/zmq_io.h>
#include <duels/game_state.h>
#include <duels/parser.h>
#include <duels/player.h>

namespace duels
{

namespace
{

inline void printWinner(const std::string &name, State state)
{
    std::string why;
    switch(state)
    {
    case State::WIN_FAIR:
        why = "fair";
        break;
    case State::WIN_TIMEOUT:
        why = "timeout";
        break;
    case State::WIN_DISCONNECT:
        why = "disconnect";
        break;
    default:
        break;
    }
    std::cout << "Winner: " << name << " (" << why << ")" << std::endl;
}

// debug functions
template <class T>
void print(std::string s, T val)
{
    std::cout << "[server] " << s << " = " << val << std::endl;
}

void print(std::string s)
{
    std::cout << "[server] " << s << std::endl;
}

template <class inputMsg, class feedbackMsg>
class RemotePlayer : public Player<inputMsg, feedbackMsg>
{
public:
    inline RemotePlayer(PlayerType type) : Player<inputMsg, feedbackMsg>(type) {}
    constexpr inline void updateInput() override {}
};
}


template <class initMsg, class inputMsg, class feedbackMsg, class displayMsg>
class Server
{
    using PlayerIO = Interface<inputMsg, feedbackMsg>;
    using PlayerPtr = std::unique_ptr<Player<inputMsg, feedbackMsg>>;

    static constexpr bool use_threads = false;

public:

    ~Server()
    {
        sock.close();
        if(p1)
            p1.reset();
        if(p2)
            p2.reset();
        ctx.close();
    }

    Server(std::string game, Timeout timeout, Refresh period) :
        sock(ctx, zmq::socket_type::pub), rate(period), timeout(timeout), game(game) {}

    Server(std::string game, Refresh refresh, Timeout timeout) : Server(game, timeout, refresh) {}

    template <class GameAI>
    std::pair<PlayerPtr, PlayerPtr>
    initPlayers(int argc, char** argv, const initMsg &init_msg, int ai1_level, int ai2_level)
    {
        static_assert(std::is_base_of<Player<inputMsg, feedbackMsg>, GameAI>::value, "Your game AI class should inherit from Player<inputMsg, feedbackMsg>");

        ArgParser parser(argc, argv);
        use_display = parser.displayPort();

        // build players depending on name or not
        std::pair<PlayerPtr, PlayerPtr> players;

        const auto &[basename1, ai1] = parser.player1(ai1_level); {}
        const auto &[basename2, ai2] = parser.player2(ai2_level); {}

        if(ai1)
        {
            players.first = std::make_unique<GameAI>(std::stoi(basename1));
            name1 = "Bot [" + basename1 + "]";
        }
        else
        {
            players.first = std::make_unique<RemotePlayer<inputMsg, feedbackMsg>>(PlayerType::RemoteOne);
            name1 = basename1;
            p1 = std::make_unique<PlayerIO>(timeout, ctx, parser.clientURL(1), use_threads);
        }

        if(ai2)
        {
            players.second = std::make_unique<GameAI>(std::stoi(basename2));
            name2 = "Bot [" + basename2 + "]";
        }
        else
        {
            players.second = std::make_unique<RemotePlayer<inputMsg, feedbackMsg>>(PlayerType::RemoteTwo);
            name2 = basename2;
            p2 = std::make_unique<PlayerIO>(timeout, ctx, parser.clientURL(2), use_threads);
        }

        // send initial display info as rep-req
        if(use_display)
        {
            std::vector<int> port_offsets;

            // if both players are local AI, run a local display
            if(!p1 && !p2)
            {
                port_offsets.push_back(1);

                // run display exec
                std::stringstream cmd;
                cmd << "python3 " << GAME_SOURCE_DIR << "/" << game << "_gui.py" << " " << DUELS_BIN_PATH
                    << " 127.0.0.1 "
                    << parser.port() + 3 << " "
                    << getpid() << " &";
                run(cmd);
            }
            else
            {
                if(p1)
                    port_offsets.push_back(1);
                if(p2)
                    port_offsets.push_back(2);
            }

            //print("Connecting to display @ " + parser.displayURL());
            const std::string req(init_msg.toYAMLString(name1,name2));
            sock.bind(parser.displayURL());

            for(auto port_offset: port_offsets)
            {
                //print("Shacking with display @ " + parser.shakeURL(port_offset));
                zmq::socket_t shake(ctx, zmq::socket_type::req);
                shake.bind(parser.shakeURL(port_offset));
                zmq::message_t zmsg(req.data(), req.length());
                shake.send(zmsg, zmq::send_flags::none);
                (void) shake.recv(zmsg);
                shake.close();
            }
            wait(100);
        }
        else
            wait(100);

        return players;
    }

    double samplingTime() const
    {
        return rate.ms.count() *0.001;
    }

    bool sync(PlayerPtr &player)
    {
        if(player->type() == PlayerType::LocalAI)
        {
            player->updateInput();
            return true;
        }

        const auto &p = (player->type() == PlayerType::RemoteOne)?p1:p2;

        p->send(player->feedback);
        const auto bond = p->waitForInput(player->input);
        return player->feedback.state == State::ONGOING && bond == Bond::OK;
    }

    bool sync(PlayerPtr &player1, PlayerPtr &player2)
    {
        /*if constexpr(use_threads)
        {
            //temptative for parallel listening - does not work
            // inform players anyway
            p1->send(msg1);
            p2->send(msg2);

            const auto status1 = p1->waitForInput(player1_input);
            const auto status2 = p2->waitForInput(player2_input);

            return msg1.state == State::ONGOING
                    && msg2.state == State::ONGOING
                    && status1 == Bond::OK
                    && status2 == Bond::OK;
        }*/

        const auto ok1(sync(player1));
        const auto ok2(sync(player2));

        return ok1 && ok2;
    }

    void registerVictory(const PlayerPtr &winner, const PlayerPtr &loser)
    {
        winner->feedback.state = State::WIN_FAIR;
        loser->feedback.state = State::LOSE_FAIR;
    }

    void sendResult(const displayMsg &display, const PlayerPtr &player1, const PlayerPtr &player2)
    {
        auto &msg1(player1->feedback);
        auto &msg2(player2->feedback);


        if(p1)
        {
            if(p1->status == Bond::TIMEOUT)
            {
                msg1.state = State::LOSE_TIMEOUT;
                msg2.state = State::WIN_TIMEOUT;
            }
            else if(p1->status == Bond::DISCONNECT)
            {
                msg2.state = State::WIN_DISCONNECT;
            }
        }

        if(p2)
        {
            if(p2->status == Bond::TIMEOUT)
            {
                msg1.state = State::WIN_TIMEOUT;
                msg2.state = State::LOSE_TIMEOUT;
            }
            else if(p2->status == Bond::DISCONNECT)
                msg1.state = State::WIN_DISCONNECT;
        }

        if(msg1.state == State::WIN_FAIR || msg1.state == State::WIN_TIMEOUT || msg1.state == State::WIN_DISCONNECT)
        {
            printWinner(name1, msg1.state);
            sendDisplay(display, msg1.state == State::WIN_FAIR ? 1 : -1);
        }
        else
        {
            printWinner(name2, msg2.state);
            sendDisplay(display, msg2.state == State::WIN_FAIR ? 2 : -2);
        }

        if(p1) p1->sendResult(msg1);
        if(p2) p2->sendResult(msg2);
    }

    void wait(uint ms) const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sendDisplay(const displayMsg &display, int winner = 0)
    {
        if(!use_display)
            return;

        rate.sleep();

        const std::string msg(display.toYAMLString(winner));
        zmq::message_t zmsg(msg.data(), msg.length());
        sock.send(zmsg, zmq::send_flags::none);
    }

private:

    std::unique_ptr<PlayerIO> p1, p2;
    zmq::context_t ctx;
    zmq::socket_t sock;
    int difficulty = 1;
    bool use_display = true;
    std::string name1, name2, game;

    Timeout timeout;
    Refresh rate;
};


}




#endif // DUELS_GAMESERVER_H
