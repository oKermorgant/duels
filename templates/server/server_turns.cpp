#include <duels/server.h>
#include <duels/<game>/msg.h>
#include <duels/<game>/<game>_ai.h>
#include <duels/<game>/mechanics.h>


using namespace duels::<game>;
using duels::Player;
using duels::Timeout;
using duels::Refresh;
using GameIO = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg>;

int main(int argc, char** argv)
{
  GameIO game_io("<game>", Refresh(<refresh>), Timeout(<timeout>));
  
  // simulation time
  [[maybe_unused]] const double dt(game_io.samplingTime());
  // single display for both players
  displayMsg display;

  // TODO prepare game state / init message (for display)
  <Game>Mechanics mechanics;
  initMsg init = mechanics.initGame();

  // inform players and get whether they are remote or local AI
  auto [player1, player2] = game_io.initPlayers<<Game>AI>(argc, argv, init, 1, 1); {}


  bool player1_turn(true);
  while(true)
  {
    // adapt to who is playing
    auto &current(player1_turn ? player1 : player2);
    auto &opponent(player1_turn ? player2 : player1);

    // extract feedback for the current player
    mechanics.buildPlayerFeedback(current->feedback, player1_turn);

    // get their action / exit if they have crashed or timeout
    if(!game_io.sync(current))
      break;

    // TODO update game mechanics (display, winning conditions) from current->input
    // up to you!

    game_io.sendDisplay(display);

    // TODO check if any regular winner after this turn
    if(false)
    {
      //if(current player wins)
      game_io.registerVictory(current, opponent);
      //else
      game_io.registerVictory(opponent, current);
      break;
    }

    // switch player
    player1_turn = !player1_turn;
  }

  // send final result to players and display
  game_io.sendResult(display, player1, player2);
}
