#define LOCAL_GAME  // to test the game AI with a dumb player AI

#include <duels/<game>/msg.h>
#ifdef LOCAL_GAME
#include <duels/local.h>
#else
#include <duels/server.h>
#endif

using namespace duels::<game>;
using duels::Player;
using duels::Timeout;
using duels::Refresh;
#ifdef LOCAL_GAME
using GameIO = duels::LocalGame<initMsg, inputMsg, feedbackMsg, displayMsg>;
#else
using GameIO = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg>;
#endif

int main(int argc, char** argv)
{
  feedbackMsg feedback1, feedback2;
  inputMsg input;
  initMsg init;
  displayMsg display;
  GameIO game_io(Timeout(<timeout>), Refresh(<refresh>));
  // simulation time
  const double dt(game_io.samplingTime());

  // build initial game state
  // prepare init message / game state


#ifdef LOCAL_GAME
  game_io.initDisplay(init, "<game>");
  game_io.setLevel(1);
#else
  game_io.initDisplay(argc, argv, init);
  const bool two_players = game_io.hasTwoPlayers();
#endif

  Player player;
  while(true)
  {
    // build current player feedback (feedback1 or feedback2)



    // request player 1 action
    if(player.isPlayerOne())
    {
#ifndef LOCAL_GAME
      // sync with player 1, exits if needed
      if(!game_io.sync(Player::One, feedback1, input))
        break;
#else
      // TODO write dumb player AI from feedback1 to input

      
      
      
      
      
      
#endif
    }
#ifndef LOCAL_GAME
    else if(two_players)
    {
      // sync with player 2, exits if needed
      if(!game_io.sync(Player::Two, feedback2, input))
        break;
    }
#endif
    else
    {
      // TODO game AI goes here: compute input from feedback2

        
        
        

    }

    // update game state according to input
    // update / send display accordingly


    game_io.sendDisplay(display);



    // check if any regular winner after this turn
    if(false)
    {
      //if(current player wins)
      game_io.registerVictory(player, feedback1, feedback2);
      //else
      game_io.registerVictory(player.opponent(), feedback1, feedback2);
      break;
    }
    
    // switch player
    player.change();
  }

  game_io.sendResult(display, feedback1, feedback2);
}
