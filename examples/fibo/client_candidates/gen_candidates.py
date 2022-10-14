#!/usr/bin/python3
from subprocess import run
import yaml
import os
from shutil import copy
import sys

base_dir = os.path.abspath('.')
build_dir = os.path.abspath(os.path.dirname(__file__) + '/build')

elos = {'players': [{'name': 'Bot [1]', 'elos': [1200], 'exec': None, 'pos': 0}]}

os.chdir(build_dir)

reasons = ('timeout', 'crash', 'lose')
times = list(range(2, 7, 2))
#reasons = ('lose',)
#times = [4, 6]
pos = 1

build = '-b' in sys.argv

for reason in reasons:
    for t in times:
        bot_name = '{} @ {}'.format(reason, t).title()
        exec_name = '{}_{}'.format(reason, t)
        
        if build:
            print('Compiling {}...'.format(exec_name))
            run(['cmake','..','-D', 'CMAKE_CXX_FLAGS=-DFORCE_{}={}'.format(reason.upper(), t)])
            run(['make'])
            copy('fibo', exec_name)
        
        elos['players'].append({'name': bot_name, 'elos': [1200], 'exec': build_dir + '/' + exec_name, 'pos': pos})
        pos += 1
        
with open(base_dir + '/fibo.yaml', 'w') as f:
    yaml.safe_dump(elos, f)
    
os.chdir(base_dir)
