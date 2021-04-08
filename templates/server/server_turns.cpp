#include <duels/server.h>
#include <duels/<game>/msg.h>
#include <duels/<game>/<game>_ai.h>
#include <duels/<game>/mechanics.h>


using namespace duels::<game>;
using duels::Result;
using duels::Timeout;
using duels::Refresh;
using GameIO = duels::Server<InitDisplay, Input, Feedback, Display>;

int main(int argc, char** argv)
{
  GameIO game_io("<game>", Refresh(<refresh>), Timeout(<timeout>));
  
  // simulation time
  [[maybe_unused]] const double dt(game_io.samplingTime());

  // TODO prepare game state / init message (for display)
  <Game>Mechanics mechanics;
  InitDisplay init = mechanics.initGame();

  // inform displays and get players (no multithread by default for simultaneous games)
  const auto [player1, player2] = game_io.initPlayers<<Game>AI>(argc, argv, init, 0, 1, false); {}

  bool player1_turn(true);
  while(game_io.running())
  {
    // adapt to who is playing
    auto [current, opponent] = game_io.newTurn(player1_turn);

    // extract feedback for the current player
    mechanics.buildPlayerFeedback(current->feedback, player1_turn);

    // get their action / exit if they have crashed or timeout
    if(!game_io.sync(current))
      break;

    // TODO update game mechanics (display, winning conditions) from current->input
    // up to you!

    game_io.sendDisplay(mechanics.display());

    // TODO check if any regular winner after this turn
    //if(...)
    {
      /* depending on the rules, you may use:
      game_io.registerVictory(current);
      game_io.registerVictory(opponent);
      game_io.registerDraw();
      game_io.endsWith(Result::P1_WINS);
      game_io.endsWith(Result::P2_WINS);
      */
    }

    // switch player
    player1_turn = !player1_turn;
  }

  // send final result to players and display
  game_io.sendResult(mechanics.display());
}
