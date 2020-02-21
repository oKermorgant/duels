#ifndef PODS_MSG_H
#define PODS_MSG_H
#include <game_server.h>
#include <player_client.h>
#include <sstream>
namespace game_1vs1 {
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
  template <class, class, class, int timeout>
  friend class ecn::GameServer;
  template <class, class, class V>
  friend class ecn::PlayerClient;
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
using Server = GameServer<InputMsg, FeedbackMsg, DisplayMsg, 1000>;
using Client = PlayerClient<InputMsg, FeedbackMsg, DisplayMsg>;
}
}
#endif
