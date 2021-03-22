#ifndef DUELS_SERVER_H
#define DUELS_SERVER_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <iostream>
#include <algorithm>

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
    //std::cout << "[server] " << s << " = " << val << std::endl;
}

void print(std::string s)
{
    //std::cout << "[server] " << s << std::endl;
}

}


template <class initMsg, class inputMsg, class feedbackMsg, class displayMsg>
class Server
{
    using PlayerIO = Interface<inputMsg, feedbackMsg>;

    static constexpr bool use_threads = false;

public:

    ~Server()
    {
        sock.close();
        p1.reset();
        if(hasTwoPlayers())
            p2.reset();
        ctx.close();
    }

    Server(Timeout timeout, Refresh period) :
        sock(ctx, zmq::socket_type::pub), rate(period), timeout(timeout) {}

    Server(Refresh refresh, Timeout timeout) : Server(timeout, refresh) {}

    Player player() const
    {
        return {1};
    }

    void initDisplay(int argc, char** argv, const initMsg &init_msg)
    {
        ArgParser parser(argc, argv);

        use_display = parser.displayPort();
        parser.getServerParam(name1, name2, difficulty);

        // wait for clients
        print("Connecting to player1 @ " + parser.clientURL(1));
        p1 = std::make_unique<PlayerIO>(timeout, ctx, parser.clientURL(1), use_threads && hasTwoPlayers());
        if(hasTwoPlayers())
        {
            print("Connecting to player2 @ " + parser.clientURL(2));
            p2 = std::make_unique<PlayerIO>(timeout, ctx, parser.clientURL(2), use_threads);
        }

        // send initial display info as rep-req
        if(use_display)
        {
            print("Connecting to display @ " + parser.displayURL());
            const std::string req(init_msg.toYAMLString(name1,name2));
            sock.bind(parser.displayURL());
            for(int player = 1; player < (hasTwoPlayers()?3:2); ++player)
            {
                print("Connecting shacking with display @ " + parser.shakeURL(player));
                zmq::socket_t shake(ctx, zmq::socket_type::req);
                shake.bind(parser.shakeURL(player));
                zmq::message_t zmsg(req.data(), req.length());
                shake.send(zmsg, zmq::send_flags::none);
                (void) shake.recv(zmsg);
                shake.close();
            }
            wait(100);
        }
        else
            wait(100);
    }

    double samplingTime() const
    {
        return rate.ms.count() *0.001;
    }

    bool hasTwoPlayers() const
    {
        return difficulty == 0;
    }

    int level() const
    {
        return difficulty;
    }

    bool sync(const Player &player, const feedbackMsg &msg, inputMsg &player_input)
    {
        print("Sending state to ", player.which());
        const auto &p = player.isPlayerOne()?p1:p2;
        p->send(msg);

        print("feedback sent -> msg.state = OnGoing", msg.state == State::ONGOING);
        const auto bond = p->waitForInput(player_input);
        print("Got their input ", player.isPlayerOne() ? "p1" : "p2");
        print("bond OK", bond == Bond::OK);
        return msg.state == State::ONGOING && bond == Bond::OK;
    }

    bool sync(const feedbackMsg &msg1, inputMsg &player1_input,
              const feedbackMsg &msg2, inputMsg &player2_input)
    {
        if constexpr(use_threads)
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
        }

        const auto ok1(sync(Player::One, msg1, player1_input));
        const auto ok2(sync(Player::Two, msg2, player2_input));

        return ok1 && ok2;
    }

    void registerVictory(const Player &winner, feedbackMsg &msg1, feedbackMsg &msg2)
    {
        msg1.state = State::WIN_FAIR;
        msg2.state = State::LOSE_FAIR;

        if(winner.isPlayerTwo())
            std::swap(msg1.state, msg2.state);
    }

    void sendResult(const displayMsg &display, feedbackMsg &msg1, feedbackMsg &msg2)
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

        if(hasTwoPlayers())
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

        p1->sendResult(msg1);
        if(hasTwoPlayers())
            p2->sendResult(msg2);
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
    std::string name1, name2;

    Timeout timeout;
    Refresh rate;
};


}




#endif // DUELS_GAMESERVER_H