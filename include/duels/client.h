#ifndef DUELS_CLIENT_H
#define DUELS_CLIENT_H

#include <iostream>
#include <duels/zmq_io.h>
#include <duels/game_state.h>
#include <duels/parser.h>
#include <unistd.h>

namespace duels
{
namespace
{
template <class T>
void print(std::string s, T val={})
{
   // std::cout << "[" << current_time("client") << "] " << s << " = " << val << std::endl;
}
void print(std::string s)
{
  //  std::cout << "[" << current_time("client") << "] " << s << std::endl;
}
}

template <class inputMsg, class feedbackMsg>
class Client
{
private:
    inputMsg input;
    feedbackMsg feedback;
    zmq::context_t ctx;
    zmq::socket_t sock;
    Timeout server_timeout;
    Timeout timeout;

    Clock::time_point feedback_time;
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
        const std::string server_dir(DUELS_BIN_PATH);
        const std::string gui_dir(DUELS_BIN_PATH);
#endif

        if(parser.localServer())
        {
            (void)system(("killall " + game + "_server -q").c_str());

            // launch local game
            std::stringstream ss;
            ss << server_dir << "/" << game << "_server";
            ss << " -p " << parser.port();
            ss << " -n1 '" << name << "'";
            ss << " -d " << difficulty;
            if(!parser.displayPort())
                ss << " --nodisplay";
            ss << " &";
            (void)system(ss.str().c_str());
        }

        // open display for this game
        if(parser.displayPort())
        {
            std::stringstream ss;
            ss << "python3 " << gui_dir << "/" << game << "_gui.py"
               << " " << DUELS_BIN_PATH
               << " " << parser.serverIP()
               << " " << parser.displayPort()
               << " " << pid << " &";
            (void)system(ss.str().c_str());
        }

        // connect to server as REP
        print("Connecting to server @ " + parser.serverURL());
        sock.setsockopt( ZMQ_LINGER, 0 );
        sock.connect(parser.serverURL());
    }

    bool get(feedbackMsg &msg)
    {
        static bool first_contact(true);

        print("waiting feedback", first_contact);
        if(first_contact)
        {
            // no timeout
            zmq::message_t zmsg;
            (void)sock.recv(zmsg);
            msg = *(static_cast<feedbackMsg*>(zmsg.data()));
            first_contact = false;
        }
        else if(read_timeout(sock, msg, server_timeout) == Bond::DISCONNECT)
            msg.state = State::SERVER_DISCONNECT;

        if(msg.state != State::ONGOING)
        {
            switch (msg.state)
            {
            case State::WIN_FAIR:
                std::cout << "You win! Fair victory" << std::endl;
                break;
            case State::WIN_TIMEOUT:
                std::cout << "You win! Opponent has timed out" << std::endl;
                break;
            case State::WIN_DISCONNECT:
                std::cout << "You win! Opponent has crashed" << std::endl;
                break;
            case State::LOSE_FAIR:
                std::cout << "You lose! Fair game" << std::endl;
                break;
            case State::LOSE_TIMEOUT:
                std::cout << "You lose! Timed out..." << std::endl;
                break;                
            default:
                std::cout << "Game has stopped - very long timeout from you or your opponent";
                if(looks_like_timeout)
                    std::cout << " (it looks like it could be you)";
                std::cout << std::endl;
                break;
            }
            return false;
        }
        feedback_time = Clock::now();
        return true;
    }

    void send(const inputMsg &msg)
    {
        print("sending input");
        if(timeout.tooLongSince(feedback_time))
            looks_like_timeout = true;
        send_timeout(sock, msg, timeout);
    }

};

}

#endif // DUELS_CLIENT_H
