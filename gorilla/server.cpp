#include <duels/gorilla/msg.h>
#include <duels/server.h>
#include <gorilla.h>

using duels::Player;
using namespace duels::gorilla;
using Game = duels::Server<InitMsg, InputMsg, FeedbackMsg, DisplayMsg, 1000, 20>;

int main(int argc, char** argv)
{
  FeedbackMsg feedback1, feedback2;
  InitMsg init;
  InputMsg input;
  DisplayMsg display;

  // prepare init message, may depend on requested difficulty
  const auto level = Game::parseLevel(argc, argv);
  Gorilla simulation(init, level);

  Game game(argc, argv, display, init);
  const bool two_players = game.hasTwoPlayers();

  // simulation time
  const uint dt_ms(100);

  // build initial game state
  display.x = 1;
  display.y = -1;
  display.hit = false;

  while(simulation.nextTurn(display))
  {
    game.sendDisplay();

    if(simulation.p1_turn)
    {
      if(!game.sync(Player::One, feedback1, input))
        break;
    }
    else
    {
      if(two_players)
      {
        if(!game.sync(Player::Two, feedback2, input))
          break;
      }
      else
      {
        // put AI here to compute input

      }
    }

    simulation.launchBanana(input);
    while(simulation.onAir(display))
    {
      game.sendDisplay();
      game.wait(50);
    }

    simulation.writeState(simulation.p1_turn?feedback1:feedback2);
  }

  // check if any regular winner
  if(simulation.winner() == 1)
    game.registerVictory(Player::One, feedback1, feedback2);
  else if(simulation.winner() == 2)
    game.registerVictory(Player::Two, feedback1, feedback2);

  game.sendResult(feedback1, feedback2);
}
