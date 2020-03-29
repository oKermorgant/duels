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

template <typename V>
void printSrv(V msg)
{
  std::cout << "                             Server -> " << msg << std::endl;
}

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
        printSrv("Loop waiting to listen");
        std::unique_lock<std::mutex> lk(*mtx);
        cv->wait(lk, [this](){return listening;});

        printSrv("Loop waiting for input");
        read(input);
        listening = false;
        std::cout << "Loop has read input" << std::endl;
        cv->notify_one();
      }
    }

    void send(const feedbackMsg &msg)
    {
      printSrv("Sending feedback");
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
      {
        printSrv("read again before sending?");
        read(input, 2*timeout);
      }
      msg.state = State::LOSE_TIMEOUT;
      if(status != Client::DISCONNECT)
        send(msg);
    }

    Client waitForInput(inputMsg &ret)
    {
      printSrv("waitForInput");
      if(use_thread)
      {
        std::unique_lock<std::mutex> lck(*mtx);
        cv->wait(lck, [this](){return !listening;});
      }
      else
        read(ret);

      if(status == Client::OK)
        printSrv("input OK");
      else if(status == Client::TIMEOUT)
        printSrv("Timeout");
      else
        printSrv("Disconnect");
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

  static int parseLevel(int argc, char** argv)
  {
    if(argc == 1) // local game with default difficulty
      return 1;
    if(argc == 3) // local game with explicit difficulty
      return atoi(argv[2]);
    return 0;     // online game, no difficulty
  }

  Server(int argc, char** argv, displayMsg &_display, const initMsg &init_msg)
    : display(_display), sock(ctx, zmq::socket_type::pub), refresh_ms(refresh)
  {
    // register player names and base port
    // name1 [name2] [port]
    // +0 -> p1
    // +1 -> p2
    // +2 -> displays
    // +3 -> shake display 1
    // +4 -> shake display 2
    std::string name1("Player"), name2("Bot");
    difficulty = parseLevel(argc, argv);

    int port = 3000;
    if(argc > 1)  // possibly local game on port 3000
      name1 = std::string(argv[1]);

    if(argc == 4) // distant game
    {
      name2 = std::string(argv[2]);
      port = atoi(argv[3]);
      // make sure base port is player 1 port
      port -= (port % 4);
      printSrv("Two players");
    }

    // wait for clients
    p1 = std::make_unique<Listener>(ctx, tcp_transport(port), hasTwoPlayers());
    if(hasTwoPlayers())
      p2 = std::make_unique<Listener>(ctx, tcp_transport(port+1), true);

    // send initial display info as rep-req
    const std::string req(init_msg.toYAMLString(name1, name2));

    sock.bind(tcp_transport(port+2));
    printSrv("Publishing to " + tcp_transport(port+2));
    for(int i = 0; i < (hasTwoPlayers()?2:1); ++i)
    {
      zmq::socket_t shake(ctx, zmq::socket_type::req);
      shake.bind(tcp_transport(port+3+i));
      printSrv("Sending names @ " + tcp_transport(port+3+i));
      zmq::message_t zmsg(req.data(), req.length());
      shake.send(zmsg, zmq::send_flags::none);
      printSrv("waiting for display resp");
      shake.recv(zmsg);
      shake.close();
      wait(100);
    }
    printSrv("display ready");
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

  void sendResult(feedbackMsg &msg1, feedbackMsg &msg2)
  {
    if(p1->status == Client::TIMEOUT)
    {
      msg1.state = State::LOSE_TIMEOUT;
      msg2.state = State::WIN_TIMEOUT;
    }
    else if(p1->status == Client::DISCONNECT)
      msg2.state = State::WIN_DISCONNECT;
    else if(p2->status == Client::TIMEOUT)
    {
      msg1.state = State::WIN_TIMEOUT;
      msg2.state = State::LOSE_TIMEOUT;
    }
    else if(p2->status == Client::DISCONNECT)
      msg1.state = State::WIN_DISCONNECT;

    printSrv("Sending final results");
    if(msg1.state == State::WIN_FAIR || msg1.state == State::WIN_TIMEOUT || msg1.state == State::WIN_DISCONNECT)
      sendDisplay(1);
    else
      sendDisplay(2);

    p1->sendResult(msg1);
    if(hasTwoPlayers())
      p2->sendResult(msg2);
  }

  void wait(uint ms) const
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

  void sendDisplay(int winner = 0)
  {
    std::this_thread::sleep_until(refresh_last + refresh_ms);
    refresh_last = std::chrono::steady_clock::now();
    printSrv("Sending display");
    const std::string msg(display.toYAMLString(winner));
    zmq::message_t zmsg(msg.data(), msg.length());
    sock.send(zmsg, zmq::send_flags::none);
  }

private:

  std::unique_ptr<Listener> p1, p2;
  zmq::context_t ctx;
  zmq::socket_t sock;
  int difficulty = 0;
  displayMsg &display;

  std::chrono::steady_clock::time_point refresh_last = std::chrono::steady_clock::now();
  const std::chrono::milliseconds refresh_ms;
};
}




#endif // DUELS_GAMESERVER_H
