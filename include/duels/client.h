#ifndef DUELS_CLIENT_H
#define DUELS_CLIENT_H

#include <iostream>
#include <duels/zmq.hpp>
#include <duels/game_state.h>
#include <unistd.h>

namespace duels
{

template <class inputMsg, class feedbackMsg>
class Client
{
private:
    inputMsg input;
    feedbackMsg feedback;
    zmq::context_t ctx;
    zmq::socket_t sock;

public:
    const int timeout;

    Client(int _timeout, std::string name, int difficulty, std::string ip, std::string game, std::string server_args)
        : timeout(_timeout), sock(ctx, zmq::socket_type::rep)
    {
        int port(3000);

        // request game server at IP
        bool local_game = true;
        if(ip != "local_game")
        {
            zmq::socket_t manager(ctx, zmq::socket_type::req);
            manager.connect("tcp://" + ip + ":2999");

            // send game and name
            if(server_args == "")
                server_args = "_";
            std::string req(game + " " + name + " " + server_args);
            zmq::message_t zreq(req.data(), req.length());
            manager.send(zreq, zmq::send_flags::none);

            // get port back
            zmq::pollitem_t item = {static_cast<void*>(manager), 0, ZMQ_POLLIN, 0};
            zmq::poll(&item, 1, _timeout);
            zmq::message_t zrep;
            if(item.revents & ZMQ_POLLIN)
            {
                (void) manager.recv(zrep);
                std::stringstream ss(std::string(static_cast<char*>(zrep.data()), zrep.size()));
                ss >> port;
                if(port)
                    local_game = false;
                if(port % 5 == 0)
                    std::cout << "Waiting for another player on port " << port << "..." << std::endl;
                else
                    std::cout << "Joining a waiting player on port " << port << "..." << std::endl;
            }
        }

        int pid(getpid());

        // paths may change depending on local testing
#ifdef GAME_SOURCE_DIR
        const std::string server_dir(std::string(GAME_SOURCE_DIR) + "/build");
        const std::string gui_dir= (GAME_SOURCE_DIR);
#else
        const std::string server_dir(DUELS_BIN_PATH);
        const std::string gui_dir(DUELS_BIN_PATH);
#endif

        if(local_game)
        {
            ip = "127.0.0.1";
            srand(time(nullptr));
            port = 3000 + 5*(rand() % 100);
            (void)system(("killall " + game + "_server -q").c_str());

            // launch local game
            std::stringstream ss;
            ss << server_dir << "/" << game << "_server";
            ss << " -p " << port;
            ss << " -n1 '" << name << "'";
            ss << " -d " << difficulty;
            ss << " " << server_args << " &";
            (void)system(ss.str().c_str());
        }

        // open display for this game
        if(server_args.find("nodisplay") == server_args.npos)
        {
            std::stringstream ss;
            ss << "python3 " << gui_dir << "/" << game << "_gui.py"
               << " " << DUELS_BIN_PATH
               << " " << ip
               << " " << port+3
               << " " << pid << " &";
            (void)system(ss.str().c_str());
        }

        // connect to server as REP
        std::stringstream ss;
        ss << "tcp://" << ip << ":" << port;
        sock.connect(ss.str());
    }

    bool get(feedbackMsg &msg)
    {
        zmq::message_t zmsg;

        (void) sock.recv(zmsg);
        msg = *(static_cast<feedbackMsg*>(zmsg.data()));

        return msg.state == State::ONGOING;
        /*if(msg.state != State::ONGOING)
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
                break;
            }
            return false;
        }
        return true;*/
    }

    void send(const inputMsg &msg)
    {
        zmq::message_t zmsg(&msg, sizeof(msg));
        sock.send(zmsg, zmq::send_flags::none);
    }

};

}

#endif // DUELS_CLIENT_H
