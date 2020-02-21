#include <duels/server.h>
#include <duels/pods/msg.h>

using namespace duels::pods;
using Server = duels::Server<InputMsg, FeedbackMsg, DisplayMsg, 1000>;

int main(int argc, char** argv)
{
  FeedbackMsg feedback1(1,2,3), feedback2;
  InputMsg input1, input2;
  DisplayMsg display{0,0,0};

  Server server(display);
  server.initConnection(argc, argv);
  const bool second_player = server.hasSecondPlayer();

  // simulation time
  const uint dt_ms(100);



  // build initial game state


  while(true)
  {
    // check if any regular winner
    if(false)
    {
      //if(...)
      server.hasWon(server.PLAYER1, feedback1, feedback2);
      //else
      server.hasWon(server.PLAYER2, feedback1, feedback2);
    }


    // build display information


    // build player 1 feedback


    // build player 2 feedback



    // sync with player 1, exits if needed
    if(!server.sync(server.PLAYER1, feedback1, input1))
      break;

    // sync with player 1, exits if needed
    if(second_player)
    {
      if(!server.sync(server.PLAYER2, feedback2, input2))
        break;
    }
    else
    {
      // artificial opponent: put your AI here


    }


    // update game state from input1 and input2


    server.wait(dt_ms);
  }

  server.sendResult(feedback1, feedback2);
}
