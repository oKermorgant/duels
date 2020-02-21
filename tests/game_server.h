#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <vector>
#include <chrono>
#include <thread>
#include <string.h>
#include <iostream>
#include <game_status.h>

namespace game_1vs1
{
/*
namespace
{

template <class InputMsg>
struct Listener
{
  bool listen = false;
  bool received = false;
  const int sock;
  const int wait;
  std::thread thread;
  std::atomic<InputMsg> msg;
  std::chrono::milliseconds t0;
  float msg_time;

  Listener(int _sock, int _wait, const std::chrono::milliseconds &t) : sock(_sock), wait(_wait), t0(t)
  {
    std::cout << "Launching thread" << std::endl;
    thread = std::thread(&Listener::loop, this);
    std::cout << "Running thread" << std::endl;
    //thread.detach();
  }

  void loop()
  {
    try
    {
      while(true)
      {
        {
          //send(sock, "waiting", 8, 0);
          std::cout << "Thread waiting for read" << std::endl;
          read(sock, &msg, sizeof(msg));
          const auto now =
          msg_time = std::chrono::duration< double >
              (std::chrono::system_clock::now());

          std::cout << "msg received" << std::endl;
        //  listen = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));
      }
    }
    catch (...) {} // error in parsing, will timeout
  }
};
}*/

template <class InputMsg, class FeedbackMsg, class DisplayMsg, int _timeout>
class GameServer
{
public:

  typedef enum Player {PLAYER1, PLAYER2} Player;
  typedef enum PlayerStatus {STATUS_OK, STATUS_TIMEOUT, STATUS_DISCONNECT} PlayerStatus;

  GameServer() : timeout{0, 1000*_timeout} {}

  void initConnection(ushort port, bool second_player = false)
  {
    if(second_player)
    {
      initSocket(port+1, socket2, server2);
      // listeners.push_back(Listener<InputMsg>(socket2, timeout/10));
    }
    else
    {
      initSocket(port, socket1, server1);
      //  listeners.push_back(Listener<InputMsg>(socket1, timeout/10));
    }
  }

  PlayerStatus sync(const Player &player,
                    const FeedbackMsg &msg,
                    const DisplayMsg &display,
                    InputMsg &player_input)
  {
    int sock = (player == PLAYER1?socket1:socket2);
    // inform player anyway
    sendPlayerState(sock, msg, display);

    if(msg.status == ONGOING)
      return readPlayerInput(sock, player_input);
    else
      return STATUS_DISCONNECT;
  }

  void stop()
  {
    send(socket1, "stop", 4, 0);
  }

  void wait(uint ms) const
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

private:
  int server1, socket1, server2, socket2;
  timeval timeout;
  //std::vector<Listener<InputMsg>> listeners;


  void initSocket(ushort port, int &_socket, int &_server)
  {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((_server = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
      perror("socket failed");
      exit(EXIT_FAILURE);
    }

    if (setsockopt(_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Forcefully attaching socket to the port
    if (bind(_server, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }
    std::cout << "Waiting for client..." << std::endl;
    if (listen(_server, 3) < 0)
    {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    if ((_socket = accept(_server, (struct sockaddr *)&address,
                          (socklen_t*)&addrlen))<0)
    {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    std::cout << " ok" << std::endl;
  }

  PlayerStatus readPlayerInput(int sock, InputMsg &player_input)
  {
    std::cout << "Waiting for input" << std::endl;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    int rv = select(sock+1, &rfds, nullptr, nullptr, &timeout);
    if (rv == 0)
      return STATUS_TIMEOUT;
    else if(rv > 0)
    {
      rv = read(sock, &player_input, sizeof(player_input));
      if(rv > 1)
        return STATUS_OK;
    }
    return STATUS_DISCONNECT;
  }

  bool sendPlayerState(int sock, const FeedbackMsg &msg, const DisplayMsg &display) const
  {
    // try
    // {
    std::cout << "Sending" << std::endl;
    send(sock,  &msg, sizeof(msg), 0);
    send(sock,  &display, sizeof(display), 0);
    std::cout << "ok" << std::endl;
    /*  return true;
    }
    catch(...)
    {
      return false;
    }*/
  }
};


}




#endif // GAMESERVER_H
