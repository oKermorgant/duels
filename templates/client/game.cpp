#include <duels/<game>/game.h>

using namespace duels::<game>;

int main(int argc, char** argv)
{
  Game game(argc, argv, "your name");

  inputMsg input;
  feedbackMsg feedback;
  const auto timeout = game.timeout;

  while(game.get(feedback))
  {
    // write input in less than timeout



    game.send(input);
  }
}
