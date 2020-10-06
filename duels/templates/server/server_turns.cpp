#define LOCAL_GAME  // to test the game AI with a dumb player AI

#include <duels/<game>/msg.h>
#ifdef LOCAL_GAME
#include <duels/local.h>
#else
#include <duels/server.h>
#endif
#include <duels/<game>/mechanics.h>
#include <duels/utils/rand_utils.h>

using duels::Player;
using namespace duels::<game>;
#ifdef LOCAL_GAME
using GameIO = duels::LocalGame<initMsg, inputMsg, feedbackMsg, displayMsg, <timeout>, <refresh>>;
#else
using GameIO = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg, <timeout>, <refresh>>;
#endif


int main(int argc, char** argv)
{
  feedbackMsg feedback1, feedback2;
  inputMsg input;
  displayMsg display;
  GameIO game_io;
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

  int turn(2);
  const std::array<Player, 2> players{Player::One, Player::Two};
  while(true)
  {
    // switch player
    turn = 3-turn;

    // build current player feedback



    // request player action
    if(turn == 1)
    {
#ifndef LOCAL_GAME
      // sync with player 1, exits if needed
      if(!game_io.sync(Player::One, feedback1, input))
        break;
#else
      // write dumb player AI from feedback1 to input

      
      
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
      // game AI goes here: compute input from feedback2


    }

    // update game state according to input
    // update / send display accordingly


    game_io.sendDisplay(display);



    // check if any regular winner after this turn
    if(game.hit_type)
    {
      //if(...)
      game_io.registerVictory(Player::One, feedback1, feedback2);
      //else
      game_io.registerVictory(Player::Two, feedback1, feedback2);
      break;
    }
  }

  game_io.sendResult(display, feedback1, feedback2);
}
