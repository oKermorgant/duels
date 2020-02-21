#ifndef GORILLA_MSG_H
#define GORILLA_MSG_H
#include <sstream>
#include <duels/game_state.h>
namespace duels {
namespace gorilla {
struct InitMsg
{
  int x1; int y1; int x2; int y2; int yb[640]; int radius;
  std::string toYAMLString(std::string p1, std::string p2) const 
  {
    std::stringstream ss;
    ss << "p1: " << p1;
    ss << "\np2: " << p2;
    ss << "\nx1: " << x1;
    ss << "\ny1: " << y1;
    ss << "\nx2: " << x2;
    ss << "\ny2: " << y2;
    ss << "\nyb: " << "[";
    for(size_t i=0; i < 640; ++i)
      ss << yb[i] << (i == 639?"]":", ");
    ss << "\nradius: " << radius;
    return ss.str();
  }
};

struct InputMsg
{
  float angle; float force;
};

struct FeedbackMsg
{
  int x; int y; int xo; int yo; int xb; int yb; int wind;
  FeedbackMsg() {}
  FeedbackMsg(int _x, int _y, int _xo, int _yo, int _xb, int _yb, int _wind)
    : x(_x), y(_y), xo(_xo), yo(_yo), xb(_xb), yb(_yb), wind(_wind) {}
  State state = State::ONGOING;
};

struct DisplayMsg
{
  float x; float y; bool hit;
  std::string toYAMLString(int winner) const 
  {
    std::stringstream ss;
    ss << "winner: " << winner;
    ss << "\nx: " << x;
    ss << "\ny: " << y;
    ss << "\nhit: " << hit;
    return ss.str();
  }
};

}
}
#endif
