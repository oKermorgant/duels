#ifndef DUELS_ZMQIO_H
#define DUELS_ZMQIO_H

#include <string>
#include <iostream>
#include <thread>
#include <ctime>
#include <duels/zmq.hpp>
#include <duels/game_state.h>

namespace duels
{

using Clock = std::chrono::steady_clock;

#ifdef DUELS_SERVER
struct Refresh
{
    explicit Refresh(int ms) : ms(ms) {}
    std::chrono::milliseconds ms;
    Clock::time_point last = Clock::now();
    inline void sleep()
    {
        std::this_thread::sleep_until(last + ms);
        last = Clock::now();
    }
};
#endif

struct Timeout
{
    explicit Timeout(int ms) : ms(ms) {}
    const std::chrono::milliseconds ms;
    inline bool tooLongSince(const Clock::time_point & start, const Clock::time_point & end = Clock::now()) const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start) > ms;
    }
    inline auto margin() const {return 2*ms;}
};

inline std::string current_time()
{
    const auto now(std::chrono::system_clock::now());
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm(std::localtime(&time));
    char buffer[26];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S.", tm);
    std::stringstream ss;
    ss << buffer;

    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    // remove microsecond cast line if you would like a higher resolution sub second time representation, then just stream remaining.count()
    auto micro = std::chrono::duration_cast<std::chrono::microseconds>(remaining);
    ss << micro.count();
    return ss.str();
}

inline std::string current_time(std::string s)
{
    return s + " @ " + current_time();
}

template <class Msg>
void read_raw(zmq::socket_t &sock, Msg &msg)
{
  zmq::message_t zmsg;
  (void)sock.recv(zmsg);
  msg.deserialize({zmsg.data<const char>(), zmsg.size()});
}

template <class Msg>
Bond read_timeout(zmq::socket_t &sock, Msg &msg, const Timeout & timeout, const Clock::time_point &start = Clock::now())
{
    using std::chrono::milliseconds;
    zmq::pollitem_t poll_in{nullptr, 0, ZMQ_POLLIN, 0};
    poll_in.socket = static_cast<void*>(sock);

    zmq::poll(&poll_in, 1, timeout.margin());

    if(!(poll_in.revents & ZMQ_POLLIN))
        return Bond::DISCONNECT;

    read_raw(sock, msg);

    return timeout.tooLongSince(start) ? Bond::TIMEOUT : Bond::OK;
}

template <class Msg>
Bond send_timeout(zmq::socket_t &sock, const Msg &msg, const Timeout &timeout)
{
    static zmq::pollitem_t poll_out{nullptr, 0, ZMQ_POLLOUT, 0};
    poll_out.socket = static_cast<void*>(sock);
    zmq::poll(&poll_out, 1, timeout.ms);

    if(!(poll_out.revents & ZMQ_POLLOUT))
        return Bond::DISCONNECT;

    // serialize and send
    const std::string ser(msg.serialize());
    zmq::message_t zmsg(ser.data(), ser.length());
    sock.send(zmsg, zmq::send_flags::none);
    return Bond::OK;
}

}

#endif // DUELS_ZMQIO_H
