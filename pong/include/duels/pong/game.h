#ifndef PONG_GAME_H
#define PONG_GAME_H
#include <duels/client.h>
#include <duels/pong/msg.h>
#include <sstream>
namespace duels {
namespace pong {
class Game: public duels::Client<inputMsg, feedbackMsg>
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
      : duels::Client<inputMsg, feedbackMsg>(
      100, name, difficulty, ip, "pong",
      "/home/olivier/code/projects/duels/duels/bin/") {}
};
}
}
#endif