#include <duels/<game>/msg.h>
#include <duels/server.h>
#include <duels/<game>/<game>_ai.h>
#include <duels/<game>/mechanics.h>

using namespace duels::<game>;
using duels::Player;
using duels::Timeout;
using duels::Refresh;
using GameIO = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg>;


int main(int argc, char** argv)
{
  GameIO game_io("<game>", Timeout(<timeout>), Refresh(<refresh>));
  
  // single display for both players
  displayMsg display;

  // TODO prepare game state / init message (for display)
  <Game>Mechanics mechanics;
  initMsg init = mechanics.initGame();

  // inform players and get whether they are remote or local AI
  auto [player1, player2] = game_io.initPlayers<<Game>AI>(argc, argv, init, 1, 1); {}
  // simulation time
  [[maybe_unused]] const double dt(game_io.samplingTime());
  

  while(true)
  {
    // TODO check if any regular winner
    if(false)
    {
      //if(player 1 wins)
      game_io.registerVictory(player1, player2);
      //else
      game_io.registerVictory(player2, player1);
    }


    // TODO build display information

    game_io.sendDisplay(display);
    
    // extract feedbacks
    mechanics.buildPlayerFeedbacks(player1->feedback, player2->feedback);
    
    // request player actions, exits if any disconnect / crash
      if(!game_io.sync(player1, player2))
        break;

    // TODO update game state from player1->input and player2->input
    
    
    
    
  }

  // final results
  game_io.sendResult(display, player1, player2);
}
