#ifndef DUELS_CLIENT_H
#define DUELS_CLIENT_H

#include <iostream>
#include <duels/zmq.hpp>
#include <duels/game_state.h>

namespace duels
{

namespace {
void print(std::string msg)
{
  std::cout << "Client -> " << msg << std::endl;
}}

template <class InputMsg, class FeedbackMsg>
class Client
{
private:
  InputMsg input;
  FeedbackMsg feedback;
  zmq::context_t ctx;
  zmq::socket_t sock;

public:
  const int timeout;

  Client(int _timeout, std::string name, int difficulty, std::string ip, std::string game, std::string duels_bin)
    : timeout(_timeout), sock(ctx, zmq::socket_type::rep)
  {
    int port(3000);

    // request game server at IP
    bool local_game = true;
    if(ip != "127.0.0.1")
    {
      zmq::socket_t manager(ctx, zmq::socket_type::req);
      manager.connect("tcp://" + ip + ":2999");

      // send game and name
      std::string req(game + " '" + name + "'");
      zmq::message_t zreq(req.data(), req.length());
      manager.send(zreq, zmq::send_flags::none);

      // get port back
      zmq::pollitem_t item = {static_cast<void*>(manager), 0, ZMQ_POLLIN, 0};
      zmq::poll(&item, 1, _timeout);
      zmq::message_t zrep;
      if(item.revents & ZMQ_POLLIN)
      {
        manager.recv(zrep);
        local_game = false;
        std::stringstream ss(std::string(static_cast<char*>(zrep.data()), zrep.size()));
        ss >> port;
      }
    }

    if(local_game)
    {
      ip = "127.0.0.1";
      port = 3000;

      // launch local game
      std::stringstream ss;
      ss << duels_bin << game << "_server " << name << " " << difficulty << " &";
      system(ss.str().c_str());
    }

    // open display for this game
    std::stringstream ss;
    ss << duels_bin << game << "_gui" << " " << ip << " " << port+3 << " &";
    //print("Running " + ss.str());
    //system(ss.str().c_str());

    // connect to server as REP
    ss.str("");
    ss << "tcp://" << ip << ":" << port;
    sock.connect(ss.str());
  }

  bool get(FeedbackMsg &msg)
  {
    print("waiting for feedback");
    zmq::message_t zmsg;
    sock.recv(zmsg);
    msg = *(static_cast<FeedbackMsg*>(zmsg.data()));
    if(msg.state == State::ONGOING) print("feedback = State::ONGOING");
    if(msg.state == State::LOSE_TIMEOUT) print("feedback = TIMEOUT");

    return msg.state == State::ONGOING;
  }

  void send(const InputMsg &msg)
  {
    print("sending input");
    zmq::message_t zmsg(&msg, sizeof(msg));
    sock.send(zmsg, zmq::send_flags::none);
  }

  void printResult()
  {


  }
};


}

#endif // DUELS_CLIENT_H
