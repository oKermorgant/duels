#include "pods.h"

int main()
{
  ecn::pods::Client client;
  client.initConnection(8080);

  ecn::pods::InputMsg input;
  ecn::pods::FeedbackMsg feedback;

  uint it(0);
  while(client.get(feedback))
  {
    it++;
    std::cout << "Reading feedback " << feedback.x << " " << feedback.y << std::endl;
    input.v = 1;
    input.w = 0.5;
    //std::cout << "Sending " << input.toString() << std::endl;
    std::cout << "Sending input" << std::endl;

    if(it == 5)
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    client.send(input);
  }

}
