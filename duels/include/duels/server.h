#ifndef DUELS_SERVER_H
#define DUELS_SERVER_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <condition_variable>
#include <duels/game_state.h>
#include <duels/zmq.hpp>

namespace duels
{

namespace
{

enum class Client{OK, TIMEOUT, DISCONNECT};

std::string tcp_transport(int port = 0)
{
    std::stringstream ss;
    ss << "tcp://*:" << port;
    return ss.str();
}
}

enum class Player {One, Two};

template <class initMsg, class inputMsg, class feedbackMsg, class displayMsg, int timeout, int refresh>
class Server
{
    struct Listener
    {
    public:
        inputMsg input;
        Client status = Client::OK;

        std::unique_ptr<std::condition_variable> cv;
        std::unique_ptr<std::mutex> mtx;
        bool listening = false;
        std::unique_ptr<std::thread> thr;

        bool use_thread;
        zmq::socket_t sock;
        zmq::pollitem_t poll_in, poll_out;
        std::chrono::steady_clock::time_point start;

        Listener(const Listener&) = delete;
        Listener(zmq::context_t &ctx, const std::string &transport, bool _use_thread = true)
            : use_thread(_use_thread), sock(ctx, zmq::socket_type::req)
        {
            sock.bind(transport);
            poll_in = {static_cast<void*>(sock), 0, ZMQ_POLLIN, 0};
            poll_out = {static_cast<void*>(sock), 0, ZMQ_POLLOUT, 0};

            if(use_thread)
            {
                cv = std::make_unique<std::condition_variable>();
                mtx = std::make_unique<std::mutex>();
                thr = std::make_unique<std::thread>(&Listener::loop, this);
            }
        }

        ~Listener()
        {
            if(use_thread)
                thr->join();
            sock.close();
        }

        void loop()
        {
            while(status == Client::OK)
            {
                // wait for listening notification
                std::unique_lock<std::mutex> lk(*mtx);
                cv->wait(lk, [this](){return listening;});

                read(input);
                listening = false;
                cv->notify_one();
            }
        }

        void send(const feedbackMsg &msg)
        {
            zmq::message_t zmsg(&msg, sizeof(msg));
            zmq::poll(&poll_out, 1, timeout);
            if(!(poll_out.revents & ZMQ_POLLOUT))
            {
                status = Client::DISCONNECT;
                return;
            }

            sock.send(zmsg, zmq::send_flags::none);
            start = std::chrono::steady_clock::now();

            if(use_thread)
            {
                listening = true;
                cv->notify_one();
            }
        }

        void read(inputMsg &msg, int _timeout = 2*timeout)
        {
            zmq::message_t zmsg;
            zmq::poll(&poll_in, 1, _timeout);
            if(poll_in.revents & ZMQ_POLLIN)
            {
                sock.recv(zmsg);
                const auto end = std::chrono::steady_clock::now();
                if(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                        > timeout)
                    status = Client::TIMEOUT;
                msg = *(static_cast<inputMsg*>(zmsg.data()));
            }
            else
                status = Client::DISCONNECT;
        }

        void sendResult(feedbackMsg &msg)
        {
            // try to read if misdetected disconnect
            if(status == Client::DISCONNECT)
                read(input, 2*timeout);

            msg.state = State::LOSE_TIMEOUT;
            if(status != Client::DISCONNECT)
                send(msg);
        }

        Client waitForInput(inputMsg &ret)
        {
            if(use_thread)
            {
                std::unique_lock<std::mutex> lck(*mtx);
                cv->wait(lck, [this](){return !listening;});
            }
            else
                read(ret);

            return status;
        }
    };


public:

    ~Server()
    {
        sock.close();
        p1.reset();
        if(hasTwoPlayers())
            p2.reset();
        ctx.close();
    }

    int parseArgs(int argc, char** argv)
    {
        name1 = "Player";
        int port(3000);

        for(int arg = 0; arg < argc; arg++)
        {
            const std::string key(argv[arg]);
            if(key == "-d")
                difficulty = atoi(argv[++arg]);
            else if(key == "-n1")
                name1 = argv[++arg];
            else if(key == "-n2")
            {
                difficulty = 0;
                name2 = argv[++arg];
            }
            else if(key == "-p")
                port = atoi(argv[++arg]);
            else if(key == "--nodisplay")
                use_display = false;
        }

        if(difficulty)
            name2 = "Bot [" + std::to_string(difficulty) + "]";

        return port - (port % 5);
    }

    Server() : sock(ctx, zmq::socket_type::pub), refresh_ms(refresh) {}

    void initDisplay(int argc, char** argv, const initMsg &init_msg)
    {
        // register player names and base port
        // -n1 name1 [-n2 name2] [-p port] [-d difficulty] [--nodisplay]
        // +0 -> p1 in
        // +1 -> p2 in
        // +2 -> displays out
        // +3 -> shake display 1 in
        // +4 -> shake display 2 in
        auto port = parseArgs(argc, argv); {}

        // wait for clients
        p1 = std::make_unique<Listener>(ctx, tcp_transport(port), hasTwoPlayers());
        if(hasTwoPlayers())
            p2 = std::make_unique<Listener>(ctx, tcp_transport(port+1), true);

        // send initial display info as rep-req
        const std::string req(init_msg.toYAMLString(
                                  use_display ? name1 :"nodisplay",
                                  use_display ? name2 : "nodisplay"));

        sock.bind(tcp_transport(port+2));
        for(int i = 0; i < (hasTwoPlayers()?2:1); ++i)
        {
            zmq::socket_t shake(ctx, zmq::socket_type::req);
            shake.bind(tcp_transport(port+3+i));
            zmq::message_t zmsg(req.data(), req.length());
            shake.send(zmsg, zmq::send_flags::none);
            shake.recv(zmsg);
            shake.close();
            if(use_display)
                wait(100);
        }
    }

    double samplingTime() const
    {
        return refresh*0.001;
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
        const auto &p = (player == Player::One)?p1:p2;
        p->send(msg);
        const auto state = p->waitForInput(player_input);
        return msg.state == State::ONGOING && state == Client::OK;
    }

    bool sync(const feedbackMsg &msg, inputMsg &player_input)
    {
        return sync(Player::One, msg, player_input);
    }

    bool sync(const feedbackMsg &msg1, inputMsg &player1_input,
              const feedbackMsg &msg2, inputMsg &player2_input)
    {
        // inform players anyway
        p1->send(msg1);
        p2->send(msg2);

        const auto status1 = p1->waitForInput(player1_input);
        const auto status2 = p2->waitForInput(player2_input);

        return msg1.state == State::ONGOING
                && msg2.state == State::ONGOING
                && status1 == Client::OK
                && status2 == Client::OK;
    }

    void registerVictory(const Player &player, feedbackMsg &msg1, feedbackMsg &msg2)
    {
        if(player == Player::One)
        {
            msg1.state = State::WIN_FAIR;
            msg2.state = State::LOSE_FAIR;
        }
        else
        {
            msg2.state = State::WIN_FAIR;
            msg1.state = State::LOSE_FAIR;
        }
    }

    void sendResult(const displayMsg &display, feedbackMsg &msg1, feedbackMsg &msg2)
    {
        if(p1->status == Client::TIMEOUT)
        {
            msg1.state = State::LOSE_TIMEOUT;
            msg2.state = State::WIN_TIMEOUT;
        }
        else if(p1->status == Client::DISCONNECT)
            msg2.state = State::WIN_DISCONNECT;

        if(hasTwoPlayers())
        {
            if(p2->status == Client::TIMEOUT)
            {
                msg1.state = State::WIN_TIMEOUT;
                msg2.state = State::LOSE_TIMEOUT;
            }
            else if(p2->status == Client::DISCONNECT)
                msg1.state = State::WIN_DISCONNECT;
        }

        if(msg1.state == State::WIN_FAIR || msg1.state == State::WIN_TIMEOUT || msg1.state == State::WIN_DISCONNECT)
        {
            if(!use_display)
                std::cout << name1 << " " << winMsg(msg1.state) << std::endl;
            sendDisplay(display, 1);
        }
        else
        {
            if(!use_display)
                std::cout << name2 << " " << winMsg(msg1.state) << std::endl;
            sendDisplay(display, 2);
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
        std::this_thread::sleep_until(refresh_last + refresh_ms);
        refresh_last = std::chrono::steady_clock::now();
        const std::string msg(display.toYAMLString(winner));
        zmq::message_t zmsg(msg.data(), msg.length());
        sock.send(zmsg, zmq::send_flags::none);
    }

private:

    std::unique_ptr<Listener> p1, p2;
    zmq::context_t ctx;
    zmq::socket_t sock;
    int difficulty = 1;
    bool use_display = true;
    std::string name1, name2;

    std::chrono::steady_clock::time_point refresh_last = std::chrono::steady_clock::now();
    const std::chrono::milliseconds refresh_ms;
};
}




#endif // DUELS_GAMESERVER_H
