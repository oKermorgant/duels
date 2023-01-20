#ifndef DUELS_PLAYER_IO_H
#define DUELS_PLAYER_IO_H

#include <ostream>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <duels/zmq_io.h>
#include <duels/game_state.h>
#include <duels/game_result.h>
#include <duels/player.h>

namespace duels
{
using Clock = std::chrono::steady_clock;

template <class Input, class Feedback>
class RemotePlayer : public Player<Input, Feedback>
{
    using Player<Input, Feedback>::feedback;
    using Player<Input, Feedback>::input;
    zmq::socket_t sock;
    Timeout &timeout;

public:
    RemotePlayer(const RemotePlayer&) = delete;
    RemotePlayer(Timeout &timeout, zmq::context_t &ctx, const std::string &transport)
        : sock(ctx, zmq::socket_type::req), timeout(timeout)
    {
        sock.bind(transport);
    }

    ~RemotePlayer()
    {
        sock.close();
    }

    // overrides
    inline void updateInput() override
    {
        const auto bond1(send_timeout(sock, feedback, timeout));
        const auto bond2(read_timeout(sock, input, timeout));
        feedback.__state.bond = result::worstOf(bond1, bond2);
    }
};


enum class PlayerType{Remote, Local};
enum class PlayerTurn{WAIT, PLAY, STOP};

template <class Input, class Feedback>
class PlayerIO
{

public:
    std::unique_ptr<Player<Input, Feedback>> ai;
    std::string name;
    Timeout timeout;

    inline PlayerIO(Timeout timeout) : timeout(timeout) {}
    ~PlayerIO()
    {
        if(thr.joinable())
            thr.join();
    }

    template <class LocalAI>
    inline void initLocal(std::string difficulty, bool use_threads = true)
    {
        ai = std::make_unique<LocalAI>(std::stoi(difficulty));
        init("Bot [" + difficulty + "]", PlayerType::Local, use_threads);
    }

    inline void initRemote(const std::string & name, zmq::context_t &ctx, const std::string &transport, bool use_threads = true)
    {
        ai = std::make_unique<RemotePlayer<Input, Feedback>>(timeout, ctx, transport);
        init(name, PlayerType::Remote, use_threads);
    }

    inline PlayerType type() const {return player_type;}
    inline bool isRemote() const {return player_type == PlayerType::Remote;}

    // access underlying state
    inline State state() const {return ai->feedback.__state;}
    inline State & state() {return ai->feedback.__state;}
    inline bool stillInGame() const {return state().stillInGame();}

    inline virtual void carryOn(PlayerTurn turn = PlayerTurn::PLAY)
    {
        this->turn = turn;
        cv.notify_all();
    }

    inline virtual void waitForInput()
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [&](){return turn != PlayerTurn::PLAY;});
    }

    inline bool play()
    {
        if(player_type == PlayerType::Local)
        {
            start = std::chrono::steady_clock::now();
            if(turn == PlayerTurn::STOP)
                return true;
            ai->updateInput();
            state().set(timeout.tooLongSince(start) ? Bond::TIMEOUT : Bond::OK);
        }
        else
        {
            ai->updateInput();
            if(turn == PlayerTurn::STOP)
                return true;
        }
        return false;
    }

private:
    PlayerType player_type;

    inline void init(std::string name, PlayerType type, bool use_threads = true)
    {
        this->name = name;
        player_type = type;

        if(use_threads)
            thr = std::thread(&PlayerIO::loop, this);
    }

    // player thread in server
    std::condition_variable cv;
    std::mutex mtx;
    std::thread thr;
    PlayerTurn turn = PlayerTurn::WAIT;
    std::chrono::steady_clock::time_point start;

    void loop()
    {
        while(true)
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv.wait(lck, [&](){return turn != PlayerTurn::WAIT;});

            if(play())
                break;

            carryOn(PlayerTurn::WAIT);
        }
    }
};


}

#endif // DUELS_PLAYER_IO_H
