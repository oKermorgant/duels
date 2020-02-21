#ifndef PODS_MSG_H
#define PODS_MSG_H
#include <sstream>
#include <duels/server.h>
#include <duels/client.h>
namespace duels {
namespace pods {
struct InputMsg
{
  float v; float w;
};
struct FeedbackMsg
{
  float x; float y; float t;
  FeedbackMsg() {}
  FeedbackMsg(float _x, float _y, float _t)
    : x(_x), y(_y), t(_t) {}
private:
  Status status = ONGOING;
  template <class, class, class, int>
  friend class duels::Server;
  template <class, class, class>
  friend class duels::Client;
};
struct DisplayMsg
{
  float x1; float y1; float t1;
  std::string toString()
  {
    std::stringstream ss;
    ss << "x1 " << x1
       << " y1 " << y1
       << " t1 " << t1;
    return ss.str();
  }
};
}
}
#endif
