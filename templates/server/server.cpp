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
  initMsg init;
  inputMsg input1, input2;
  displayMsg display;
  GameIO game_io(Timeout(<timeout>), Refresh(<refresh>));
  // simulation time
  const double dt(game_io.samplingTime());

  // build initial game state

  
  

  // build init message for display
  

#ifdef LOCAL_GAME
  game_io.initDisplay(init, "<game>");  // add false at the end if you run the display in another terminal
  game_io.setLevel(1);
#else
  game_io.initDisplay(argc, argv, init);
  const bool two_players = game_io.hasTwoPlayers();
#endif


  while(true)
  {
    // check if any regular winner
    if(false)
    {
      //if(...)
      game_io.registerVictory(Player::One, feedback1, feedback2);
      //else
      game_io.registerVictory(Player::Two, feedback1, feedback2);
    }


    // build display information

    game_io.sendDisplay(display);
    
    // build player 1 feedback


    // build player 2 feedback




#ifndef LOCAL_GAME
    if(two_players)
    {
      if(!game_io.sync(feedback1, input1, feedback2, input2))
        break;
    }
    else
    {
      // sync with player 1, exits if needed
      if(!game_io.sync(feedback1, input1))
        break;


#else
      // write dumb player AI from feedback1 to input1
      
      
      
      


#endif

      // artificial opponent: put your AI here
      
      
      
      
      
      
      

#ifndef LOCAL_GAME
    }
#endif

    // update game state from input1 and input2
    
    
    
    
  }

  // final results
  game_io.sendResult(display, feedback1, feedback2);
}
