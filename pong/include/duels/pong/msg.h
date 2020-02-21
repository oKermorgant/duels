#ifndef PONG_MSG_H
#define PONG_MSG_H
#include <sstream>
#include <duels/game_state.h>
namespace duels {
namespace pong {
struct InitMsg
{
  ;
  std::string toYAMLString(std::string p1, std::string p2) const 
  {
    std::stringstream ss;
    ss << "p1: " << p1;
    ss << "\np2: " << p2;
    return ss.str();
  }
};

struct InputMsg
{
  float v;
};

struct FeedbackMsg
{
  float x1; float x2; float x; float y;
  FeedbackMsg() {}
  FeedbackMsg(float _x1, float _x2, float _x, float _y)
    : x1(_x1), x2(_x2), x(_x), y(_y) {}
  State state = State::ONGOING;
};

struct DisplayMsg
{
  float x1; float x2; float x; float y;
  std::string toYAMLString(int winner) const 
  {
    std::stringstream ss;
    ss << "winner: " << winner;
    ss << "\nx1: " << x1;
    ss << "\nx2: " << x2;
    ss << "\nx: " << x;
    ss << "\ny: " << y;
    return ss.str();
  }
};

}
}
#endif