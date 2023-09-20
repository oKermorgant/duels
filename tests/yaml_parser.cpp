#include <duels/game_state.h>
#include <iostream>

using namespace duels;

int main()
{
  std::string yaml{"pose: {x: 27,y: 3,orientation: 1}\nscan: []\ntreasure_distance: 19\n__state: {result: 0, bond: 0}"};

  const auto node{YAML::Load(yaml)};
  //auto __state = node["__state"].as<State>();
  auto __state = node["__state"]; //.as<State>();

  auto i{0};

   std::cout << node["__state"]["bond"].as<std::string>();


}
