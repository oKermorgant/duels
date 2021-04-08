#include <duels/fibo/msg.h>
#include <duels/server.h>
#include <duels/fibo/fibo_ai.h>
#include <duels/fibo/mechanics.h>

using namespace duels::fibo;
using duels::Timeout;
using duels::Refresh;
using GameIO = duels::Server<InitDisplay, Input, Feedback, Display>;

int main(int argc, char** argv)
{
  GameIO game_io("fibo", Refresh(500), Timeout(100));
  // simulation time
  [[maybe_unused]] const double dt(game_io.samplingTime());
  // single display for both players
  Display display;

  // build initial game state
  FiboMechanics mechanics;
  // prepare init message / game state
  InitDisplay init = mechanics.initGame();

  // inform players and get whether they are remote or local AI
  const auto [player1, player2] = game_io.initPlayers<FiboAI>(argc, argv, init, 1, 2, false); {}

  bool player1_turn(true);

  while(true)
  {
    // adapt to who is playing
    auto [current, opponent] = game_io.newTurn(player1_turn);

    // send feedback to this player
    mechanics.informPlayer(current->feedback, player1_turn);

    // get their action / exit if they have crashed or timeout
    if(!game_io.sync(current))
      break;

    // tell the game mechanics about the player input
    bool correct(mechanics.registerInput(current->input, player1_turn));

    // update game mechanics (display, winning conditions) from current->input and player1_turn
    // up to you!

    game_io.sendDisplay(display);

    if(!correct)
    {
      game_io.registerVictory(opponent);
      break;
    }

    // switch player
    player1_turn = !player1_turn;
  }

  game_io.sendResult(display);
}
