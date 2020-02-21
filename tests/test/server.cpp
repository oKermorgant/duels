#include <pods.h>
#include <math.h>

int main()
{
  ecn::pods::Server server;
  server.initConnection(8080);

  float x(0), y(0), v(0), w(0), t(0);
  const float dt(0.05f);
  const uint dt_ms(static_cast<uint>(100));

  ecn::pods::FeedbackMsg feedback(1,2,3);
  ecn::pods::InputMsg input;
  ecn::pods::DisplayMsg display{0,0,0};
  ecn::pods::Server::PlayerStatus input_status;

  while(true)
  {
    // update display

    //std::cout << "Sending to player " << feedback.toString() << std::endl;
    //std::cout << "Sending to display " << display.toString() << std::endl;
    input_status = server.sync(server.PLAYER1, feedback, display, input);



    if()
      std::cout << "timeout / crash for player 1" << std::endl;

    v = input.v;
    w = input.w;
    std::cout << "Reading input " << v << " " << w << std::endl;

    x += v*cos(t) * dt;
    y += v*sin(t) * dt;
    t += w*dt;
    feedback.x = display.x1 = x;
    feedback.y = display.y1 = y;
    feedback.t = display.t1 = t;

    server.wait(dt_ms);
  }

  std::cout << "Winner is player 2\n";
}
