#!/usr/bin/env python3
import sys
import yaml
import os

class Info:
    def __init__(self, key):
        self.type, key = key.split()
        self.name = key.split('[')[0]
        self.dim = 0
        self.arr = ''
        if '[' in key:
            self.arr = key[key.index('['):]
            self.dim = int(key[key.index('[')+1:key.index(']')])
    def ss(self, nl):
        ret = 'ss << "{}{}: "'.format(nl and '\\n' or '', self.name)
        if not self.dim:
            ret += ' << {};'.format(self.name)
        else:
            loop = ''' << "[";
    for(size_t i=0; i < {}; ++i)
      ss << {}[i] << (i == {}?"]":", ");'''
            ret += loop.format(self.dim, self.name, self.dim-1)
        return ret
            
def build_toYAMLString(detail, more = []):
    ret = '  std::string toYAMLString({}) const \n  {{\n    std::stringstream ss;\n    '.format(', '.join(more))
    for i,key in enumerate([Info(key) for key in more] + detail):
        if i:
            ret += '\n    '
        ret += key.ss(i)
    return ret + '\n    return ss.str();\n  }\n'
            
def msg_derived(name, keys, field):
    
    keys = [key.replace('(','[').replace(')',']') for key in keys]
    l = len(keys)
    detail = [Info(key) for key in keys]
    ret = 'struct {}\n{{\n'.format(name)
    if len(keys):
        ret += '  {};\n'.format('; '.join(keys))
    if field == 'display':
        ret += build_toYAMLString(detail, ['int winner'])
    if field == 'init':
        ret += build_toYAMLString(detail, ['std::string p1', 'std::string p2'])
        
    elif field == 'feedback':
        ret += '  {}() {{}}\n'.format(name)
        ret += '  {}({})\n'.format(name, ', '.join(['{} _{}{}'.format(key.type, key.name, key.arr) for key in detail]))
        ret += '    : {} {{}}\n'.format(', '.join(['{key}(_{key})'.format(key=key.name) for key in detail if key.dim == 0]))
        ret += '  State state = State::ONGOING;\n'
    return ret + '};\n'

def build_headers(game, description, game_path):

    include_path = game_path + 'include/duels/'+game
    guard = game.upper() + '_MSG_H'

    header = ['#ifndef {}'.format(guard)]
    header.append('#define {}'.format(guard))
    for inc in ('sstream', 'duels/game_state.h'):
        header.append('#include <{}>'.format(inc)) 
    header.append('namespace duels {{\nnamespace {} {{'.format(game.lower()))
    
    names = []
    for field in ('init', 'input', 'feedback', 'display'):
        names.append(field+'Msg')
        header.append(msg_derived(names[-1], description[field], field))
        
    header.append('}\n}\n#endif')
    with open(include_path + '/msg.h', 'w') as f:
        f.write('\n'.join(header))
        
    # generate client.h
    guard = game.upper() + '_GAME_H'
    header = '''#ifndef {guard}
#define {guard}
#include <duels/client.h>
#include <duels/{game}/msg.h>
#include <sstream>
namespace duels {{
namespace {game} {{
class Game: public duels::Client<inputMsg, feedbackMsg>
{{
public:
  Game(std::string name = "Player")
    : Game(name, 1, "127.0.0.1") {{}}
  Game(std::string name, int difficulty)
    : Game(name, difficulty, "127.0.0.1") {{}}
  Game(std::string name, std::string ip)
      : Game(name, 1, ip) {{}}
private:
  Game(std::string name, int difficulty, std::string ip)
      : duels::Client<inputMsg, feedbackMsg>(
      {timeout}, name, difficulty, ip, "{game}") {{}}
}};
}}
}}
#endif'''
    with open(include_path + '/game.h', 'w') as f:
        f.write(header.format(guard=guard, game=game, timeout=description['timeout']))
        
def dict_replace(s, d):
    for key in d:
        s = s.replace('<'+key+'>', str(d[key]))
    return s

if __name__ == '__main__':
    path = os.path.abspath(os.path.dirname(__file__)) + '/'
    if not os.path.exists(path + 'bin'):
        print('Run this script from an installation folder, not the source folder')
        sys.exit(0)
        
    game_path = len(sys.argv) == 2 and sys.argv[1] or '.'
    game_path = os.path.abspath(game_path) + '/'

    description_file = ''
    for msg in os.listdir(game_path):
        if msg.endswith('.yaml'):
            description_file = game_path + msg
            game = msg.split('.')[0].lower()
            break
        
    if description_file == '':
        print('Could not find any game description file in {}'.format(game_path))
        sys.exit(0)
    
    with open(description_file) as f:
        description = yaml.safe_load(f)
    if 'timeout' not in description:
        description['timeout'] = 100
    if 'refresh' not in description:
        description['refresh'] = description['timeout']
    if 'turn_based' not in description:
        description['turn_based'] = False
    description['game'] = game
    description['duels_path'] = path[:-1]
        
    # create directories
    for d in ('include', 'include/duels', 'include/duels/'+game, 'client_template'):
        if not os.path.exists(game_path + d):
            os.mkdir(game_path + d)
            
    build_headers(game, description, game_path)
    
    # copy server templates
    for src in ('CMakeLists.txt', description['turn_based'] and 'server_turns.cpp' or 'server.cpp', 'gui.py'):
        
        dst_path =  game_path + src
        
        if src == 'gui.py':
            dst_path = game_path + game + '_gui.py'
        elif src == 'server_turns.cpp':
            dst_path = game_path + 'server.cpp'
            
        if os.path.exists(dst_path):
            print('Skipping {}, file exists'.format(dst_path))
        else:
            with open(path + 'templates/server/' + src) as f:
                content = f.read()
            with open(dst_path, 'w') as f:
                f.write(dict_replace(content, description))
                
    # copy client templates
    for src,dst in (('CMakeLists.txt','CMakeLists.txt'), ('game.cpp', game+'.cpp')):
        dst_path = game_path + 'client_template/' + dst
        if os.path.exists(dst_path):
            print('Skipping {}, file exists'.format(dst_path))
        else:
            with open(path + 'templates/client/' + src) as f:
                content = f.read()
            with open(dst_path, 'w') as f:
                f.write(dict_replace(content, description))
