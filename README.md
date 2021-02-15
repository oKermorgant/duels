# Duels framework

Ongoing work for basic 1 vs 1 programming contests.

This repository contains the code generation scripts and several C++ header files (mostly netcode) to have people (e.g. students) develop AI's for various games. Games can be "played" against the internal AI of against another person, through a local network.

# Denomination

This framework uses up to 3 programs when playing a game:
- the `server` is the game itself and includes a default AI with possibly several difficulty levels
- the `client` is the user (or player) code, that receives the game state and sends back its input
- the `display` is in Python and runs on the user computer. It receives display data from the game but does not send anything

During development, the `client` can be embedded into the `server` to easily create the mechanics, rules and display.

# Creating a new game

## Generation of boilerplate files

For the framework, a duel game is defined by a yaml file (see examples) stating four kinds of messages:
- init: what should be sent by the game at initialization
- input: the expected player input
- feedback: the feedback from the game to a player
- display: what is required to display the state (not accessible to the player)
Optionally, these fields can also be 

To create a new game:
- create a folder with the name of the game
- create a yaml file with the same name, that contains the message information
- call python <path_to_installed_duels>/bin/gen_wrapper.py <game_name>

It will create the core `server.cpp` file where the game mechanics and display should be defined. 
It also creates a `client_template` directory, to be used by actual game player

## Developing the game

The `server.cpp` is the entry point for the game server. By default, it is generated with `#define LOCAL_GAME` that allows testing the game AI against a simulated player AI that may be as dumb or clever as you want. Actually playing against this game can only be done if this `define` is disabled.

