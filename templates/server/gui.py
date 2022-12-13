#!/usr/bin/env python3
import sys
<enums_py>

msg_from_cpp = True  # make it False to allow running the gui with use manually-written messages

if msg_from_cpp:

    # normal behavior
    sys.path.insert(1, sys.argv[1])
    from duels import Subscriber
    game = Subscriber(<server_timeout>)
    init_msg = game.get_init()

else:
    # messages are written manually
    class dict_to_obj(object):
        def __init__(self, adict):
            self.__dict__.update(adict)

    def to_object(msg):
        if isinstance(msg, list):
            return [to_object(v) for v in msg]
        elif isinstance(msg, dict):
            for v in msg:
                msg[v] = to_object(msg[v])
            return dict_to_obj(msg)
        return msg

    init_msg = <init_msg_py>




# add other imports here /(e.g. pygame) 



# prepare initial state / display

'''

'''

# init_msg.name1
# init_msg.name2
# init_msg.<other fields>

if msg_from_cpp:
    game.ready()

while True:

    if msg_from_cpp:
        msg = game.refresh()
        if game.winner:
            break
    else:
        # write msg manually
        msg = <msg_py>
    
    # update display from msg fields
    
    
    
# update display from game.winner (1 or 2, 3 for draw or -1 if any bug that got you there)
# if winner is 1 or 2 you can use game.winner_name(init_msg) to get their name
