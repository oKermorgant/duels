#!/usr/bin/env python3
import time
import zmq
import os
from subprocess import Popen, PIPE
import threading


# get known games
base_path = os.path.dirname(__file__)

class Game:
    def __init__(self, game, name1, port, server_args):
        self.game = game
        self.port = port
        self.name1 = name1
        self.name2 = None
        self.process = None
        self.server_args = server_args
        
    def running(self):
        return self.name2 is not None
    
    def register_player(self, game, name2):
        if game == self.game and not self.running():
            self.name2 = name2
            # run the game server with these specs
            self.process = Popen(['{}/{}_server'.format(base_path, game),
                                  '-n1', self.name1,
                                  '-n2', self.name2,
                                  '-p', str(self.port),
                                  server_args,'&'], shell = False, stdout=PIPE , stderr=PIPE)
            return True
        return False
    
    def output(self):
        if self.process is None:
            return 'not running'
        if self.process.poll() is None:
            return 'done'
        try:
            return self.process.communicate(timeout = .1)[0].decode('utf-8')
        except:
            return ""
    
    def has_ended(self):
        return self.process is not None and self.process.poll() is None
    
    def description(self):
        return '{} with {} vs {}'.format(self.game,self.name1,self.name2)


known_games = [game.replace('_server','') for game in os.listdir(base_path) if game.endswith('_server')]

games = []

def monitor_games():
    global games
    while True:
        idx = 0
        while idx < len(games):
            popped = False
            #if games[idx].running():
            if True:
                out = games[idx].output()
                print(games[idx].description() + ': "' + out + '"')
                if out == 'done':
                    games[idx].process.terminate()
                    games.pop(idx)
                    popped = True
            if not popped:
                idx += 1                    
        time.sleep(1)
        

monitor = threading.Thread(target = monitor_games)
monitor.start()

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:2999")

while True:
    message = socket.recv().decode('utf8').split()
       
    game = message[0]
    server_args = message[-1]
    name = ' '.join(message[1:-1])
    
    print('{} wants to play {} with args "{}"'.format(name,game,server_args))
       
    port = 0
    if game in known_games:
        joining = False
        
        # look for a waiting player on this game
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
            games.append(Game(game, name, port, server_args))            
    
    # inform client about their connection port
    socket.send_string(str(port))
    
    
    
    
