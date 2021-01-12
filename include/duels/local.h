#ifndef DUELS_LOCAL_H
#define DUELS_LOCAL_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <algorithm>
#include <condition_variable>
#include <duels/game_state.h>
#include <duels/zmq.hpp>
#include <unistd.h>
#include <iostream>

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

template <class initMsg, class inputMsg, class feedbackMsg, class displayMsg,
          int timeout, int refresh>
class LocalGame
{
public:

    ~LocalGame()
    {
        sock.close();
        ctx.close();
    }

    void setLevel(int level)
    {
        difficulty = level;
    }

    int level() const
    {
        return difficulty;
    }

    LocalGame() : sock(ctx, zmq::socket_type::pub), refresh_ms(refresh) {}

    void initDisplay(const initMsg &init_msg, const std::string &game, bool run_display = true)
    {
        // base port similar to server
        // +2 -> display out
        // +3 -> display 1 in
        const int port = 3000;

        if(run_display)
        {
            // run display exec
            std::stringstream ss;
            ss << "python3 " << DUELS_ROOT << "/bin/" << game << "_gui.py" << " " << DUELS_ROOT
               << " 127.0.0.1 "
               << port+3 << " "
               << getpid() << " &";
            (void)system(ss.str().c_str());
        }

        // send initial display info as rep-req
        const std::string req(init_msg.toYAMLString("Player_AI", "Game_AI [" + std::to_string(difficulty) + "]"));

        sock.bind(tcp_transport(port+2));
        zmq::socket_t shake(ctx, zmq::socket_type::req);
        shake.bind(tcp_transport(port+3));
        zmq::message_t zmsg(req.data(), req.length());
        shake.send(zmsg, zmq::send_flags::none);
        shake.recv(zmsg);
        shake.close();
        wait(100);
    }

    double samplingTime() const
    {
        return refresh*0.001;
    }

    void wait(uint ms) const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
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

    void sendDisplay(const displayMsg &display_msg, int winner = 0)
    {
        std::this_thread::sleep_until(refresh_last + refresh_ms);
        refresh_last = std::chrono::steady_clock::now();
        const std::string msg(display_msg.toYAMLString(winner));
        zmq::message_t zmsg(msg.data(), msg.length());
        sock.send(zmsg, zmq::send_flags::none);
    }

    void sendResult(const displayMsg &display, feedbackMsg &msg1, feedbackMsg &msg2)
    {
        if(msg1.state == State::WIN_FAIR || msg1.state == State::WIN_TIMEOUT || msg1.state == State::WIN_DISCONNECT)
            sendDisplay(display, 1);
        else
            sendDisplay(display, 2);
    }

private:

    zmq::context_t ctx;
    zmq::socket_t sock;
    int difficulty = 0;

    std::chrono::steady_clock::time_point refresh_last = std::chrono::steady_clock::now();
    const std::chrono::milliseconds refresh_ms;
};
}

#endif // DUELS_LOCAL_H
