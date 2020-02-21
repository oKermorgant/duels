#include "game_msg.h"
#include <iostream>

int main()
{

  ecn::GameMsg msg({"x", "y"});

  std::cout << msg.toString() << std::endl;

  std::string received("x: -1\ny: 5");
  //msg.parse(received);
  std::cout << msg.toString() << std::endl;

}
