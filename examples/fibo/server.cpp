#include <duels/fibo/msg.h>
#include <duels/server.h>
#include <duels/fibo/fibo_ai.h>
#include <duels/fibo/mechanics.h>

using namespace duels::fibo;
using duels::Timeout;
using duels::Refresh;
using GameIO = duels::Server<initMsg, inputMsg, feedbackMsg, displayMsg>;

int main(int argc, char** argv)
{
  GameIO game_io("fibo", Refresh(1000), Timeout(100));
  // simulation time
  [[maybe_unused]] const double dt(game_io.samplingTime());
  // single display for both players
  displayMsg display;

  // build initial game state
  FiboMechanics mechanics;
  // prepare init message / game state
  initMsg init = mechanics.initGame();

  // inform players and get whether they are remote or local AI
  auto [player1, player2] = game_io.initPlayers<FiboAI>(argc, argv, init, 1, 1); {}

  bool player1_turn(true);

  while(true)
  {
    // adapt to who is playing
    auto &current(player1_turn ? player1 : player2);
    auto &opponent(player1_turn ? player2 : player1);

    // send feedback to this player
    mechanics.informPlayer(current->feedback, player1_turn);

    // get their action / exit if they have crashed or timeout
    if(!game_io.sync(current))
      break;

    // tell the game mechanics about the player input
    mechanics.registerInput(current->input, player1_turn);

    // update game mechanics (display, winning conditions) from current->input and player1_turn
    // up to you!

    game_io.sendDisplay(display);

    std::cout << current->input.s << std::endl;

    if(current->feedback.a + current->feedback.b != current->input.s)
    {
      game_io.registerVictory(opponent, current);
      break;
    }

    // switch player
    player1_turn = !player1_turn;
  }

  game_io.sendResult(display, player1, player2);
}
