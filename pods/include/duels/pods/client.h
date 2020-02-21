#ifndef PODS_CLIENT_H
#define PODS_CLIENT_H
#include <duels/client.h>
#include <duels/pods/msg.h>
#include <sstream>
namespace duels {
namespace pods {
class Client: public duels::Client<InputMsg, FeedbackMsg, DisplayMsg>
{
public:
  Client() : duels::Client<InputMsg, FeedbackMsg, DisplayMsg>(100, "/opt/duels/bin/pods_gui", "/opt/duels/bin/pods_server") {}
};
}
}
#endif
