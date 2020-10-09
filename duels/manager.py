#!/usr/bin/env python3
import time
import zmq
import os
from subprocess import Popen


# get known games
base_path = os.path.dirname(__file__) + '/bin'

class Game:
    def __init__(self, game, name1, port):
        self.game = game
        self.port = port
        self.name1 = name1
        self.name2 = None
        self.process = None
    def register_player(self, game, name2):
        if game == self.game and self.name2 is None:
            self.name2 = name2
            # run the game server with these specs
            self.process = Popen(['{}/{}_server'.format(base_path, game),
                                  '-n1', self.name1,
                                  '-n2', self.name2,
                                  '-p', str(self.port),'&'], shell = False)
            return True
        return False
    
    def has_ended(self):
        return self.process is not None and self.process.poll() is None
    
    def description(self):
        return '{} with {} vs {}'.format(self.game,self.name1,self.name2)


known_games = [game.replace('_server','') for game in os.listdir(base_path) if game.endswith('_server')]

games = []

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:2999")

while True:
    message = socket.recv().decode('utf8')
    
    game,name = message.split(' ',1)
    
    print('{} wants to play {}'.format(name,game))
    
    # refresh games
    idx = 0
    while idx < len(games):
        if games[idx].has_ended():
            print('{} has ended'.format(games[idx].description()))
            games.pop(idx)
        else:
            idx += 1
    
    port = 0
    if game in known_games:
        joining = False
        
        # look for a waiting player
        for g in games:
            if g.register_player(game, name):
                joining = True
                break
        
        if joining:
            port = g.port+1
        else:
            # find a new port for this game
            used = [g.port for g in games]
            port = 3000
            while port in used:
                port += 5
            games.append(Game(game, name, port))            
    
    # inform client about their connection port
    socket.send_string(str(port))
    
    
    
    
