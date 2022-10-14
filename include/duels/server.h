#ifndef DUELS_SERVER_H
#define DUELS_SERVER_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <filesystem>

#include <duels/zmq_io.h>
#include <duels/game_state.h>
#include <duels/game_result.h>
#include <duels/parser.h>
#include <duels/player.h>
#include <duels/player_io.h>

namespace duels
{

namespace
{

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

}


template <class InitDisplay, class Input, class Feedback, class Display>
class Server
{
    using PlayerPtr = Player<Input, Feedback>*;
    using Interface = PlayerIO<Input, Feedback>;

public:

    ~Server()
    {
        sock.close();
        ctx.close();
    }

    Server(std::string game, Timeout timeout, Refresh period) :
        players{timeout, timeout}, io1{players[0]}, io2{players[1]},
        sock(ctx, zmq::socket_type::pub), rate(period), timeout(timeout), game(game) {}

    Server(std::string game, Refresh refresh, Timeout timeout) : Server(game, timeout, refresh) {}

    template <class GameAI>
    std::pair<PlayerPtr, PlayerPtr>
    initPlayers(int argc, char** argv, const InitDisplay &init_msg, int ai1_level, int ai2_level, bool use_threads = true)
    {
        static_assert(std::is_base_of<Player<Input, Feedback>, GameAI>::value, "Your game AI class should inherit from Player<Input, Feedback>");

        ArgParser parser(argc, argv);
        use_display = parser.displayPort();
        this->use_threads = use_threads;

        // build players depending on name or not
        if(const auto &[basename1, ai1] = parser.extractPlayer1(ai1_level);ai1)
        {
            players[0].template initLocal<GameAI>(basename1, use_threads);
        }
        else
        {
            players[0].initRemote(basename1, ctx, parser.clientURL(1));
        }

        if(const auto &[basename2, ai2] = parser.extractPlayer2(ai2_level);ai2)
        {
            players[1].template initLocal<GameAI>(basename2, use_threads);
        }
        else
        {
            players[1].initRemote(basename2, ctx, parser.clientURL(2), use_threads);
        }

        // send initial display info as req-res
        if(use_display)
        {
            std::vector<int> port_offsets;

            // if both players are local AI, run a local display
            if(!io1.isRemote() && !io2.isRemote())
            {
                port_offsets.push_back(1);

                // find display exec

                // default to local version
                auto gui_path = std::string(GAME_SOURCE_DIR) + "/" + game + "_gui.py";
                std::string duels_path(DUELS_BIN_PATH);

                if(!std::filesystem::exists(gui_path))
                {
                  // to installed version
                  auto server_path = std::filesystem::absolute(argv[0]).parent_path();
                  gui_path = server_path.string() + "/" + game + "_gui.py";
                  duels_path = server_path.parent_path().parent_path().string();
                }

                // run display exec
                std::stringstream cmd;
                cmd << "python3 " << gui_path << " " << duels_path
                    << " 127.0.0.1 "
                    << parser.port() + 3 << " "
                    << getpid() << " &";
                run(cmd);
            }
            else
            {
                if(io1.isRemote())
                    port_offsets.push_back(1);
                if(io2.isRemote())
                    port_offsets.push_back(2);
            }

            //print("Connecting to display @ " + parser.displayURL());
            const std::string req(init_msg.serialize(io1.name,io2.name));
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
            wait(timeout.ms.count());
        }
        //else
          //  wait(100);

        return {p1(), p2()};
    }

    inline std::pair<PlayerPtr, PlayerPtr> newTurn(bool player1_turn) const
    {
        if(player1_turn)
            return {p1(), p2()};
        return {p2(), p1()};
    }

    double samplingTime() const
    {
        return rate.ms.count() *0.001;
    }

    inline bool running() const
    {
        return io1.stillInGame() && io2.stillInGame();
    }

    inline bool sync(PlayerPtr player)
    {
        auto &io(player == p1() ? io1 : io2);

        if(use_threads)
        {
            io.carryOn();
            io.waitForInput();
        }
        else
            io.play();
        return io.stillInGame();
    }

    inline bool syncBothPlayers()
    {
        if(use_threads)
        {
            io1.carryOn();
            io2.carryOn();

            io1.waitForInput();
            io2.waitForInput();
        }
        else
        {
            io1.play();
            io2.play();
        }
        return running();
    }

    inline void endsWith(Result result)
    {
        final_state.set(result);
        for(auto &player: players)
            player.state().set(result);
    }
    inline void endsWith(Bond bond)
    {
        final_state.set(bond);
        for(auto &player: players)
            player.state().set(bond);
    }

    inline void registerVictory(PlayerPtr winner)
    {
        endsWith(winner == p1() ? Result::P1_WINS : Result::P2_WINS);
    }

    inline void registerDraw()
    {
        endsWith(Result::DRAW);
    }

    void sendResult(const Display &display)
    {
        const auto out1(!io1.state().is(Bond::OK));
        const auto out2(!io2.state().is(Bond::OK));

        // build result if any timeout / disconnect
        if(out1 && out2)
            endsWith(Result::DRAW);
        else if(out1)
            endsWith(Result::P2_WINS);
        else if(out2)
            endsWith(Result::P1_WINS);
        endsWith(result::worstOf(io1.state().bond, io2.state().bond));

        if(use_threads)
        {
            io1.carryOn(PlayerTurn::STOP);
            io2.carryOn(PlayerTurn::STOP);
        }
        else
        {
            for(auto &io: players)
            {
                if(io.isRemote())
                    io.play();
            }
        }

        // to console for automatic rating of players
        if(!use_display)
          result::print(final_state, io1.name, io2.name);
        io1.state() = final_state;
        // to display, if any
        sendDisplay(display);
    }

    void wait(uint ms) const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sendDisplay(const Display &display)
    {
        if(!use_display)
            return;

        rate.sleep();

        const std::string msg(display.serialize(io1.state().result));
        zmq::message_t zmsg(msg.data(), msg.length());
        sock.send(zmsg, zmq::send_flags::none);
    }

private:

    zmq::context_t ctx;
    zmq::socket_t sock;
    bool use_threads = true;
    bool use_display = true;
    std::array<Interface, 2> players;
    Interface &io1;
    Interface &io2;

    std::string game;
    State final_state;

    inline PlayerPtr p1() const {return io1.ai.get();}
    inline PlayerPtr p2() const {return io2.ai.get();}

    Timeout timeout;
    Refresh rate;
};


}




#endif // DUELS_GAMESERVER_H
