#!/usr/bin/env python
import sys
sys.path.insert(1, sys.argv[1])
from duels import Subscriber
game = Subscriber()
init_msg = game.get_init() 

# add other imports here /(e.g. pygame) 



# prepare initial state / display
# init_msg.p1
# init_msg.p2
# init_msg.<other fields>


game.ready()

while True:
    msg = game.refresh()
    if msg.winner:
        break
    
    # update display from fields
    
    
    
# update display from winner

