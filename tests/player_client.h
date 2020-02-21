#ifndef PLAYERCLIENT_H
#define PLAYERCLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <game_status.h>

namespace ecn
{

template <class InputMsg, class FeedbackMsg, class DisplayMsg>
class PlayerClient
{
private:
  DisplayMsg display;
  int sock = 0;
  Status status = ONGOING;

public:
  PlayerClient()
  {
    // open display for this game
    //  system("python3 ../test/display.py &");

    // auto connect to master / launch local game
  }

  void initConnection(ushort port)
  {
    initConnection("127.0.0.1", port);
  }
  void initConnection(const char ip[] = "127.0.0.1", ushort port = 3000)
  {
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("\n Socket creation error \n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
      printf("\nInvalid address/ Address not supported \n");
    }

    while(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      std::cout << "connecting..." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << " ok" << std::endl;
  }

  bool get(FeedbackMsg &msg)
  {
    std::cout << "Waiting for info" << std::endl;
    read(sock, &msg, sizeof(msg));
    read(sock, &display, sizeof(display));
    if(msg.status == ONGOING)
      return true;
    status = msg.status;
    return false;
  }

  void send(const InputMsg &msg) const
  {
    ::send(sock, &msg, sizeof(msg), 0);
  }

  void printResult()
  {
    switch(status)
    {
    case WIN_FAIR:
      1;


    }
  }
};


}




#endif // PLAYERCLIENT_H
