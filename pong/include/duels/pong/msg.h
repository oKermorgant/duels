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
  double v;
};

struct FeedbackMsg
{
  double y1; double y2; double x; double y;
  FeedbackMsg() {}
  FeedbackMsg(double _y1, double _y2, double _x, double _y)
    : y1(_y1), y2(_y2), x(_x), y(_y) {}
  State state = State::ONGOING;
};

struct DisplayMsg
{
  double y1; double y2; double x; double y;
  std::string toYAMLString(int winner) const 
  {
    std::stringstream ss;
    ss << "winner: " << winner;
    ss << "\ny1: " << y1;
    ss << "\ny2: " << y2;
    ss << "\nx: " << x;
    ss << "\ny: " << y;
    return ss.str();
  }
};

}
}
#endif