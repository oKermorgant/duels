#ifndef DUELS_CLIENT_H
#define DUELS_CLIENT_H

#include <iostream>
#include <duels/zmq_io.h>
#include <duels/game_state.h>
#include <duels/game_result.h>
#include <duels/parser.h>
#include <unistd.h>

namespace duels
{
namespace
{
template <class T>
void print(std::string s, T val={})
{
   std::cout << "[" << current_time("client") << "] " << s << " = " << val << std::endl;
}
void print(std::string s)
{
  std::cout << "[" << current_time("client") << "] " << s << std::endl;
}
}

template <class Input, class Feedback>
class Client
{
private:
    Input input;
    Feedback feedback;
    zmq::context_t ctx;
    zmq::socket_t sock;
    Timeout server_timeout;
    Timeout timeout;

    Clock::time_point feedback_time;
    Result you_win;
    bool looks_like_timeout = false;
public:

    int timeout_ms() const {return timeout.ms.count();}

    Client(int argc, char** argv, int timeout_ms, int server_timeout_ms, std::string name, int difficulty, std::string ip, std::string game)
        : server_timeout(server_timeout_ms), timeout(timeout_ms), sock(ctx, zmq::socket_type::rep)
    {
        ArgParser parser(argc, argv, ip);

        // request game server at IP
        if(parser.isRemote())
        {
            zmq::socket_t manager(ctx, zmq::socket_type::req);
            manager.connect("tcp://" + ip + ":2999");

            // send game and name
            std::string req(game + " " + name);
            zmq::message_t zreq(req.data(), req.length());
            manager.send(zreq, zmq::send_flags::none);

            // get port back
            zmq::pollitem_t item = {static_cast<void*>(manager), 0, ZMQ_POLLIN, 0};
            zmq::poll(&item, 1, timeout.ms);
            zmq::message_t zrep;
            if(item.revents & ZMQ_POLLIN)
            {
                (void) manager.recv(zrep);
                if(parser.updateFrom({static_cast<char*>(zrep.data()), zrep.size()}))
                {
                    if(parser.isPlayer1())
                        std::cout << "Waiting for another player on port " << parser.port() << "..." << std::endl;
                    else
                        std::cout << "Joining a waiting player on port " << parser.port() << "..." << std::endl;
                }
                else
                    std::cout << "Game manager not reachable, running local game with difficulty " << difficulty << std::endl;
            }
        }
        int pid(getpid());

        // paths may change depending on local testing
#ifdef GAME_SOURCE_DIR
        const std::string server_dir(std::string(GAME_SOURCE_DIR) + "/build");
        const std::string gui_dir(GAME_SOURCE_DIR);
#else
        const std::string server_dir(std::string(DUELS_BIN_PATH) + "/" + game);
        const std::string gui_dir(server_dir);
#endif

        if(parser.localServer())
        {
            run("killall " + game + "_server -q");

            // launch local game
            std::stringstream cmd;
            cmd << server_dir << "/" << game << "_server";
            if(difficulty < 0)  // request to play as player 2
            {
                cmd << " -n2 '" << name << "'";
                cmd << " -n1 " << -difficulty;
                parser.setupPlayer2();
            }
            else
            {
                cmd << " -n1 '" << name << "'";
                cmd << " -n2 " << difficulty;
            }
            cmd << " -p " << parser.port();

            if(!parser.displayPort())
                cmd << " --nodisplay";
            cmd << " &";
            run(cmd);
        }

        // custom status messages
        you_win = parser.isPlayer1() ? Result::P1_WINS : Result::P2_WINS;

        // open display for this game
        if(parser.displayPort())
        {
            std::stringstream cmd;
            cmd << "python3 " << gui_dir << "/" << game << "_gui.py"
               << " " << DUELS_BIN_PATH
               << " " << parser.serverIP()
               << " " << parser.displayPort()
               << " " << pid << " &";
            run(cmd);
        }

        // connect to server as REP
        //print("Connecting to server @ " + parser.serverURL());
        sock.setsockopt( ZMQ_LINGER, 0 );
        sock.connect(parser.serverURL());
    }

    bool get(Feedback &msg)
    {
        static bool first_contact(true);

        if(first_contact)
        {
            // no timeout
            zmq::message_t zmsg;
            (void)sock.recv(zmsg);
            msg = *(static_cast<Feedback*>(zmsg.data()));
            first_contact = false;
        }
        else if(read_timeout(sock, msg, server_timeout) == Bond::DISCONNECT)
            msg.__state.set(Bond::DISCONNECT);

        if(result::isFinal(msg.__state, you_win, looks_like_timeout))
            return false;
        feedback_time = Clock::now();
        return true;
    }

    void send(const Input &msg)
    {
        if(timeout.tooLongSince(feedback_time))
            looks_like_timeout = true;
        send_timeout(sock, msg, timeout);
    }

};

}

#endif // DUELS_CLIENT_H
