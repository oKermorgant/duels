# Duels framework

Ongoing work for basic 1 vs 1 programming contests

# Installing

The `duels` folder has to be instanciated at a chosen location, using the script `install.sh`

# Creating a new game

For the framework, a duel game is defined by a yaml file (see examples) stating four kinds of messages:
- init: what should be sent by the game at initialization
- input: the expected player input
- feedback: the feedback from the game to a player
- display: what is required to display the state (not accessible to the player)

To create a new game:
- create a folder with the name of the game
- create a yaml file with the same name, that contains the message information
- call python <path_to_installed_duels>/gen_wrapper <game_name>

It will create the core server.cpp file where the game mechanics and display should be defined. 
It also creates a `client_template` directory, to be used to create an actual game player

