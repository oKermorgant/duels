#ifndef DUELS_CLIENT_H
#define DUELS_CLIENT_H

#include <iostream>
#include <duels/zmq.hpp>
#include <duels/game_state.h>

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

  Client(int _timeout, std::string name, int difficulty, std::string ip, std::string game)
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
      ss << DUELS_ROOT << "/bin/" << game << "_server " << name << " " << difficulty << " &";
      system(ss.str().c_str());
    }

    // open display for this game
    std::stringstream ss;
    ss << DUELS_ROOT << "/bin/" << game << "_gui.py" << " " << DUELS_ROOT << " " << ip << " " << port+3 << " &";
    system(ss.str().c_str());

    // connect to server as REP
    ss.str("");
    ss << "tcp://" << ip << ":" << port;
    sock.connect(ss.str());
  }

  bool get(feedbackMsg &msg)
  {
    zmq::message_t zmsg;
    sock.recv(zmsg);
    msg = *(static_cast<feedbackMsg*>(zmsg.data()));

    return msg.state == State::ONGOING;
  }

  void send(const inputMsg &msg)
  {
    zmq::message_t zmsg(&msg, sizeof(msg));
    sock.send(zmsg, zmq::send_flags::none);
  }

  void printResult()
  {


  }
};


}

#endif // DUELS_CLIENT_H
