#include <duels/pods/client.h>

using namespace duels::pods;

int main()
{
  Client client;
  client.initConnection(8080);

  InputMsg input;
  FeedbackMsg feedback;
  const auto timeout = client.timeout;

  while(client.get(feedback))
  {



    client.send(input);
  }

}
