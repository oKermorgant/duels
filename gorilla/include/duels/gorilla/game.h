#ifndef GORILLA_CLIENT_H
#define GORILLA_CLIENT_H
#include <duels/client.h>
#include <duels/gorilla/msg.h>
#include <sstream>
namespace duels {
namespace gorilla {
class Game: public duels::Client<InputMsg, FeedbackMsg>
{
public:
  Game(std::string name = "Player")
    : Game(name, 1, "127.0.0.1") {}
  Game(std::string name, int difficulty)
    : Game(name, difficulty, "127.0.0.1") {}
  Game(std::string name, std::string ip)
      : Game(name, 1, ip) {}
private:
  Game(std::string name, int difficulty, std::string ip)
      : duels::Client<InputMsg, FeedbackMsg>(
      100, name, difficulty, ip, "gorilla",
      "/home/olivier/code/projects/duels/opt/duels/bin/") {}
};
}
}
#endif
