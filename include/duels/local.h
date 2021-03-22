#ifndef DUELS_LOCAL_H
#define DUELS_LOCAL_H

#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <algorithm>
#include <condition_variable>
#include <duels/game_state.h>
#include <duels/zmq_io.h>
#include <duels/player.h>
#include <unistd.h>
#include <iostream>

namespace duels
{

template <class initMsg, class inputMsg, class feedbackMsg, class displayMsg>
class LocalGame
{
public:

    ~LocalGame()
    {
        sock.close();
        ctx.close();
    }

    Player player() const
    {
        return {1};
    }

    void setLevel(int level)
    {
        difficulty = level;
    }

    int level() const
    {
        return difficulty;
    }

    LocalGame(Timeout, Refresh refresh) : sock(ctx, zmq::socket_type::pub), rate(refresh) {}
    LocalGame(Refresh refresh, Timeout timeout) : LocalGame(timeout, refresh) {}

    void initDisplay(const initMsg &init_msg, const std::string &game, bool run_display = true)
    {
        // base port similar to server
        // +2 -> display out
        // +3 -> display 1 in
        const int port(3000);
        const int port_display(port+2);
        const int port_shake(port+3);

        if(run_display)
        {
            // run display exec
            std::stringstream ss;
            ss << "python3 " << GAME_SOURCE_DIR << "/" << game << "_gui.py" << " " << DUELS_BIN_PATH
               << " 127.0.0.1 "
               << port_shake << " "
               << getpid() << " &";
            (void)system(ss.str().c_str());
        }

        // send initial display info as rep-req
        const std::string req(init_msg.toYAMLString("Player_AI", "Game_AI [" + std::to_string(difficulty) + "]"));

        sock.bind("tcp://*:" + std::to_string(port_display));
        zmq::socket_t shake(ctx, zmq::socket_type::req);
        shake.bind("tcp://*:" + std::to_string(port_shake));
        zmq::message_t zmsg(req.data(), req.length());
        shake.send(zmsg, zmq::send_flags::none);
        (void) shake.recv(zmsg);
        shake.close();
        wait(100);
    }

    double samplingTime() const
    {
        return rate.ms.count()*0.001;
    }

    void wait(uint ms) const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void registerVictory(const Player &player, feedbackMsg &msg1, feedbackMsg &msg2)
    {
        if(player.isPlayerOne())
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
        rate.sleep();
        const std::string msg(display_msg.toYAMLString(winner));
        zmq::message_t zmsg(msg.data(), msg.length());
        sock.send(zmsg, zmq::send_flags::none);
    }

    void sendResult(const displayMsg &display, feedbackMsg &msg1, feedbackMsg &)
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

    Refresh rate;
};
}

#endif // DUELS_LOCAL_H
