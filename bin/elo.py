#!/usr/bin/python
import sys
import yaml
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import wait as waitFutures
from threading import current_thread, Timer
import os
from shutil import copy
from subprocess import Popen, PIPE
from random import shuffle
import argparse
from time import sleep, time

try:
    import pylab as pl
    do_plot = True
except:
    do_plot = False    
        

class RankSystem:
    
    base_value = 1200 
    min_value = 100
    
    @staticmethod
    def gain(elo, matches):
        if matches < 30:
            return 40
        return 20 if elo < 2400 else 10        
    
    @staticmethod
    def elo_update(player1, player2, W1):
        
        # compute delta for player 1
        D = min(400, max(-400, player1.elo - player2.elo))
        p = 1./(1+pow(10, -D/400))
        
        delta = W1 - p
        
        return int(RankSystem.gain(player1.elo, player1.matches)*delta), -int(RankSystem.gain(player2.elo, player2.matches)*delta)

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.description = 'Automated ranking of duels players (and bots)'

# files
parser.add_argument('--players', metavar='players', type=str, nargs='+', help='Games as <name>:=<executable>')
parser.add_argument('-f', '--file', type=str, help='Result YAML file',default='/tmp/duels_elo.yaml')
parser.add_argument('-p', '--parallel', type=int, help='How many games are run in parallel',default=4)
parser.add_argument('-t', '--timeout', type=int, help='Timeout (sec) to cancel a game and decide for a draw', default=15)
parser.add_argument('-b', '--bots', type=int, help='Highest difficulty for builtin AI', default=3)
parser.add_argument('-T', '--tournaments', type=int, help='Number of tournaments to play', default=1)
parser.add_argument('--reset', action='store_true', help='Reset all previous results', default=False)

args = parser.parse_args()

elos = {}
if os.path.exists(args.file):
    with open(args.file) as f:
        elos = yaml.safe_load(f)
        
stats = {'fair': 0, 'timeout': 0, 'disconnect': 0, 'game timeout': 0, 'mean time': .2}
if 'stats' in elos:
    if args.reset:
        elos.pop('stats')
    else:
        stats = elos.pop('stats')

def port_info(port, info):
    
    colors = ['\033[95m', '\033[94m', '\033[96m', '\033[92m', '\033[93m', '\033[91m']
    end_color = '\033[0m'

    print('{}On {}: {}{}'.format(colors[(port//5)% len(colors)], port, info, end_color))

class Player:
    def __init__(self, name):
        self.name = name
        elo = elos[name]['elo'][-1]
        self.elo = elo[-1] if isinstance(elo, list) else elo
        self.opponents = elos[name]['opponents']
        self.matches = sum(self.opponents.values())
        
        if 'exec' in elos[name]:
            self.executable = elos[name]['exec']
            self.bot_level = None
        else:
            self.bot_level = ''.join(c for c in name if c.isdigit()) if '[' in name else 0
            
    def __eq__(self, name):
        return self.name == name
            
    def new_result(self, delta, name):
        self.elo = max(self.elo + delta, RankSystem.min_value)
        self.matches += 1
        self.opponents[name] += 1
        
    def spawn_client(self, port, display):
        # running the client is not blocking
        # should be killed when the game is done to avoid waiting for timeout detection
        cmd = [self.executable, '-p', str(port)]
        if not display:
            cmd.append('--nodisplay')            
        return Popen(cmd, stdout=PIPE, stderr=PIPE)
    
def parse_result(result, name1, name2):
    
    reason = 'game timeout'
    
    if not result:
        return .5, reason
    
    p1_wins = name1 in result
    p2_wins = name2 in result
    if p1_wins or p2_wins:
        reason = result.rsplit('(')[-1][:-1]
        
    return (1+p1_wins-p2_wins)/2, reason       
        
        
class Game:
    
    def __init__(self):
        
        players_args = [('Bot [{}]'.format(i+1), None) for i in range(args.bots)]
        if args.players:
            players_args += [arg.split(':=')  for arg in args.players]    

        for player,executable in players_args:
            if player not in elos:
                elos[player] = {'elo': [RankSystem.base_value], 'opponents': {}}
                if executable:
                    elos[player]['exec'] = executable
        
        for player in elos:
            for opponent in elos:
                if opponent != player and opponent not in elos[player]['opponents']:
                        elos[player]['opponents'][opponent] = 0
        
        if args.reset:
            if os.path.exists(args.file):
                copy(args.file, args.file.replace('.', '_backup.'))
            for player in elos:
                elos[player]['elo'] = [RankSystem.base_value]
                for opponent in elos[player]['opponents']:
                    elos[player]['opponents'][opponent] = 0
            
        self.name = os.path.basename(list(elos.values())[0]['exec'])
        bin_dir =  os.path.abspath(os.path.dirname(__file__)) 
        self.server =  '{}/{}_server'.format(bin_dir, self.name)
        self.executor = ThreadPoolExecutor(args.parallel)
        self.progress = 0
        
        print('Playing {} at {}'.format(self.name, self.server))
        
        self.players = [Player(name) for name in elos]
        self.all_matches = []
        for player in self.players:
            if not player.bot_level:
                for opponent in player.opponents:
                    self.all_matches.append((player, self.find(opponent)))

                            
    def find(self, player_name):
        idx = self.players.index(player_name)
        return self.players[idx]
        
    def run_tournament(self, t = 0, max_tournaments = 1):
        shuffle(self.all_matches)
        
        self.progress = t * len(self.all_matches)
        self.all_tournaments = max_tournaments * len(self.all_matches)
        results = []
        for p1, p2 in self.all_matches:
            results.append(self.executor.submit(self.official_fight, p1, p2))        
        waitFutures(results)
        [r.result() for r in results]
        self.progress = 0
        
        # update scores
        for p in self.players:
            elos[p.name]['elo'].append(p.elo)
        
    def run_game(self, cmd):
        # running a game is blocking, returns the output of the server
        proc = Popen(cmd, stdout=PIPE, stderr=PIPE)
        
        timer = Timer(args.timeout, proc.kill)
        try:
            timer.start()
            return [line.decode('utf-8').strip() for line in proc.communicate()]
        finally:
            timer.cancel()
        return None        
        
    def official_fight(self, player1, player2):
        
        self.progress += 1
        
        p1_score, reason = parse_result(self.fight(player1, player2), player1.name, player2.name)
        stats[reason] += 1

        # update Elo's
        d1, d2 = RankSystem.elo_update(player1, player2, p1_score)

        player1.new_result(d1, player2.name)
        player2.new_result(d2, player1.name)
                     
            
    def fight(self, player1, player2, display = False):
        if isinstance(player1, str):
            return self.fight(self.find(player1), player2, display)
        if isinstance(player2, str):
            return self.fight(player1, self.find(player2), display)
                
        if player1.bot_level:
            print('Asked for a fight with {} as player 1: cancelled'.format(player1.name))
            return None, None
            
        # thread ID is used to decide for the port of the game
        thr_id = current_thread().name[-1]
        port = 3005 + (5*int(thr_id) if thr_id.isdigit() else 0)
        
        # run clients before any server / get output of server
        clients = [player1.spawn_client(port, display)]
        server_args = [self.server, '-p', str(port), '-n1', player1.name]
        
        if not player2.bot_level:
            clients.append(player2.spawn_client(port+1, display))
            server_args += ['-n2', player2.name]
        else:
            server_args += ['-d', player2.bot_level]
            
        if not display:
            server_args += ['--nodisplay']
            
        out = self.run_game(server_args)
        for c in clients:
            try:
                #port_info(port, 'Killing {}'.format(c.args[0]))
                c.kill()
            except:
                pass
        
        if self.progress:
            match = '[{} %] {} vs {} -> '.format(round(100*self.progress/self.all_tournaments, 1), player1.name, player2.name)
        else:
            match = '{} vs {} -> '.format(player1.name, player2.name)
            
        if out is not None:
            for line in out:
                if line.startswith('Winner') and line.endswith(')'):
                    port_info(port, match + line)
                    return line 
        port_info(port, match + 'timeout')
        return 'Match has timed-out'

    def write_results(self):
        
        for player in self.players:
            elos[player.name]['last_elo'] = player.elo
            
        elos['stats'] = stats
        
        with open(args.file, 'w') as f:
            yaml.safe_dump(elos, f)
            
        if do_plot and len(elos[self.players[0].name]['elo']) > 2:
            n = len(self.players)
            pl.close('all')
            fig = pl.figure(figsize=(max(7, n//3), max(4, n//3)))
            
            hist = [[pl.name]+elos[pl.name]['elo'] for pl in self.players]
            hist.sort(key = lambda x: -pl.mean(x[1:]))
            
            hist = pl.array(hist)
            names, hist = hist[:,0], pl.array(hist[:,1:], dtype=int)
            
            N = hist.shape[1]+1
            x = range(1, N)
            for i,name in enumerate(names):
                pl.plot(x, [pl.mean(hist[i][:k]) for k in range(1, N)], label='{} ({})'.format(name, int(pl.mean(hist[i]))))
            pl.gca().set_xticks(range(1,N,max(1, int(pl.ceil(N/n)))))
            pl.xlabel('Tournaments')
            pl.ylabel('Elo score')
            pl.legend(loc = 'upper left')
            fig.tight_layout()
            
            fig.savefig(os.path.splitext(args.file)[0] + '.pdf')
        
        
game = Game()        

# estimate time to run all this
n = len(game.players) - args.bots
m = n*(n-1 + n*args.bots)
if stats['mean time']:
    r = input('Estimated time: {} s, continue? [Y/n] '.format(round(m*args.tournaments*stats['mean time']/args.parallel, 1)))

if r in ('n', 'N'):
    sys.exit(0)
    
t0 = time()

played_matches = sum(stats[key] for key in stats if key != 'mean time')
# time to do all the games
dt = stats['mean time'] * played_matches

for t in range(args.tournaments):
    game.run_tournament(t, args.tournaments)
    
print('Actual time: {} s (estimated = {} s)'.format(round(time() - t0, 1), round(m*args.tournaments*stats['mean time']/args.parallel, 1)))
# additional time without multithread
dt += (time() - t0) * args.parallel
played_matches += m*args.tournaments

stats['mean time'] = dt/played_matches

game.write_results()

for p in game.players:
    print('{} : {}'.format(p.name, p.elo))
