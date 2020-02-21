#!/usr/bin/env python
import sys
sys.path.insert(1, '<duels_path>')
from duels import Subscriber

sub = Subscriber()
init_msg = sub.get_init()

# prepare initial state / display
# init_msg.p1
# init_msg.p2
# init_msg.<other fields>


sub.ready()

while True:
    msg = sub.refresh()
    if msg.winner:
        break
    
    # update display from fields
    
    
    
# update display from winner

