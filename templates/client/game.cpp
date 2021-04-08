#include <duels/<game>/game.h>

using namespace duels::<game>;

int main(int argc, char** argv)
{
  
  Game game(argc, argv, "your name", 1);    // to play as player 1 against level 1 AI
  //Game game(argc, argv, "your name", -2);    // to play as player 2 against level 2 AI

  Input input;
  Feedback feedback;
  const auto timeout = game.timeout_ms();

  while(game.get(feedback))
  {
    // write input in less than timeout



    game.send(input);
  }
}