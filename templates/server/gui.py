#!/usr/bin/env python
import sys
sys.path.insert(1, sys.argv[1])
from duels import Subscriber
game = Subscriber(<server_timeout>)
init_msg = game.get_init()

# add other imports here /(e.g. pygame) 



# prepare initial state / display

'''

'''

# init_msg.name1
# init_msg.name2
# init_msg.<other fields>


game.ready()

while True:
    msg = game.refresh()
    if game.winner:
        break
    
    # update display from msg fields
    
    
    
# update display from game.winner (1 or 2, 3 for draw or -1 if any bug that got you there)
# if winner is 1 or 2 you can use game.winner_name(init_msg) to get their name
