#include <duels/pong/msg.h>
#include <duels/server.h>

using duels::Player;
using namespace duels::pong;
using Game = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg, 100, 20>;

int main(int argc, char** argv)
{
  feedbackMsg feedback1, feedback2;
  initMsg init;
  inputMsg input1, input2;
  displayMsg display;

  // prepare init message
  

  Game game(argc, argv, display, init);
  const bool two_players = game.hasTwoPlayers();

  // simulation time
  const double dt(game.samplingTime());



  // build initial game state


  while(true)
  {
    // check if any regular winner
    if(false)
    {
      //if(...)
      game.registerVictory(Player::One, feedback1, feedback2);
      //else
      game.registerVictory(Player::One, feedback1, feedback2);
    }


    // build display information

    game.sendDisplay();
    
    // build player 1 feedback


    // build player 2 feedback




    
    if(two_players)
    {
      if(!game.sync(feedback1, input1, feedback2, input2))
        break;
    }
    else
    {
      // sync with player 1, exits if needed
      if(!game.sync(feedback1, input1))
        break;
      // artificial opponent: put your AI here


    }
    // update game state from input1 and input2
  }

  game.sendResult(feedback1, feedback2);
}
