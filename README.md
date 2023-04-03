# Duels framework

Ongoing work for basic 1 vs 1 programming contests.

This repository contains the code generation scripts and several C++ header files (mostly netcode) to have people (e.g. students) develop AI's for various games. Games can be "played" against the internal AI of against another person, through a local network.

# Denomination

This framework uses up to 3 programs when playing a game:
- the `server` is the game itself and includes a default AI with possibly several difficulty levels
- the `client` is the user (or player) code, that receives the game state and sends back its input
- the `display` is in Python and runs on the user computer. It receives display data from the game but does not send anything


# Creating a new game

## Generation of boilerplate files

For the framework, a duel game is defined by a yaml file (see examples) stating four kinds of messages:
- `init_display`: what should be sent by the game at initialization
- `input`: the expected player input
- `feedback`: the feedback from the game to a player
- `display`: what is required to display the state (not accessible to the player)

These messages are defined as a list of variables that will be the final structure of the message:
- `int x` will create a `int x` member variable
- `float x()` will create a `std::vector<float> x`
- `float x(2)` will create `std::array<float,2> x`

Built-in smart structures are also available:
- `duels::Orientation`: can be `LEFT`, `RIGHT`, `UP`, `DOWN` and has methods to rotate
- `duels::Position2D`: a `(x,y)` position with `int` coordinates
- `duels::Pose2D`: a `(x,y,Orientation)` structure with `int` coordinates, that can rotate and move

Custom structures can be defined in the yaml file, under the `structs` key (see `examples/treasure_hunt.yaml`). They may be:
- A list similar to the actual messages with known C++ types (for example `Position: [int x, int y]`)
- A list with only names, in this case a corresponding `enum class` will be generated (for example `Cell: [FREE,OCCUPIED,UNKNOWN]`)

To create a new game:
- create a folder with the name of the game
- create a yaml file with the same name, that contains the message information
- call python <path_to_installed_duels>/bin/gen_wrapper.py inside the game folder

It will create the core `server.cpp` file where the game mechanics and display should be defined.
This is done through the created `mechanics.h` and `game_ai.h` files, to write the game rules and your local AI.
It also creates a `client_template` directory, to be used by actual game player.

## Developing the game

The `server.cpp` is the entry point for the game server.  The local AI should take as argument a `difficulty` parameter. The default values for player 1 and player 2 appear in `server.cpp` when creating players.

Typically, AI level 0 should have no chance to win in order to let the player test their own AI. AI level 1 may win by chance. Higher difficulty AIs should actually try to win.
