#include <duels/fibo/game.h>
#include <duels/utils/rand_utils.h>

using namespace duels::fibo;

int main(int argc, char** argv)
{
  Game game(argc, argv, "your name");

  Input input;
  Feedback feedback;
  const auto timeout = game.timeout_ms();

  int turn(0);
  while(game.get(feedback))
  {
    turn++;
    // write input in less than timeout
    input.s = feedback.a + feedback.b;

#ifdef FORCE_LOSE
      std::cout << "Will lose in " << FORCE_LOSE-turn << std::endl;
    if(turn >= FORCE_LOSE)
      input.s = 0;
#endif

#ifdef FORCE_TIMEOUT
    std::cout << "Will timeout in " << FORCE_TIMEOUT-turn << std::endl;
    if(turn >= FORCE_TIMEOUT)
      std::this_thread::sleep_for(std::chrono::milliseconds(int(1.5*timeout)));
#endif

#ifdef FORCE_CRASH
    std::cout << "Will crash in " << FORCE_CRASH-turn << std::endl;
    if(turn >= FORCE_CRASH)
      throw std::runtime_error("I wanted to crash @ " + std::to_string(turn));
#endif

    std::cout << "Got a = " << feedback.a << ", b = " << feedback.b;
    std::cout << " -> sending input: " << input.s << std::endl;

    game.send(input);
  }
}
