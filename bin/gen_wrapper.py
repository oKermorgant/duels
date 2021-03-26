#!/usr/bin/env python3
import sys
import yaml
import os



def read_file(filename, to_lines = True):
    with open(filename) as f:
        content = f.read()
    if to_lines:
        return content.splitlines()
    return content

def write_file(filename, content, overwrite = False):
    
    exists = os.path.exists(filename)
    
    if exists:
        if not overwrite:
            print('Skipping {}, file exists'.format(filename))
            return
        else:
            print('Overwriting {}'.format(filename))
    else:
        print('Creating {}'.format(filename))
    
    with open(filename, 'w') as f:
        if isinstance(content, str):
            f.write(content)
        else:
            f.write('\n'.join(content))

        
            
def dict_replace(s, d):
    for key in d:
        if '<'+key+'>' in s:
            s = s.replace('<'+key+'>', str(d[key]))
    return s

def adapt(str_in, file_out, description, file_in = True, overwrite = False):
    if file_in:
        write_file(file_out, dict_replace(read_file(str_in, False), description), overwrite)
    else:
        write_file(file_out, dict_replace(str_in, description), overwrite)
    
    
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
            
def build_print_fct(name, detail, more = []):
    ret = '  std::string {}({}) const \n  {{\n    std::stringstream ss;\n    '.format(name, ', '.join(more))
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
        ret += build_print_fct('toYAMLString', detail, ['int winner'])
    elif field == 'init':
        ret += build_print_fct('toYAMLString', detail, ['std::string p1', 'std::string p2'])
    else:
        ret += build_print_fct('toString', detail)
        
    if field == 'feedback':
        ret += '  {}() {{}}\n'.format(name)
        ret += '  {}({})\n'.format(name, ', '.join(['{} _{}{}'.format(key.type, key.name, key.arr) for key in detail]))
        ret += '    : {} {{}}\n'.format(', '.join(['{key}(_{key})'.format(key=key.name) for key in detail if key.dim == 0]))
        ret += '  State state = State::ONGOING;\n'
    return ret + '};\n'

def build_headers(game, description, game_path):

    include_path = game_path + 'include/duels/'+game
    guard = game.upper() + '_MSG_H'

    # generate msg.h
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
    
    write_file(include_path + '/msg.h', header, overwrite=True)
        
    # generate client.h
    header = '''#ifndef <GAME>_GAME_H
#define <GAME>_GAME_H
#include <duels/client.h>
#include <duels/<game>/msg.h>
#include <sstream>
namespace duels {
namespace <game> {
class Game: public duels::Client<inputMsg, feedbackMsg>
{
public:
  Game(int argc, char** argv, std::string name, int difficulty = 1)
    : Game(argc, argv, name, difficulty, "localhost") {}
  Game(int argc, char** argv, std::string name, std::string ip, int difficulty = 1)
      : Game(argc, argv, name, difficulty, ip) {}
private:
  Game(int argc, char** argv, std::string name, int difficulty, std::string ip)
      : duels::Client<inputMsg, feedbackMsg>(
      argc, argv, <timeout>, <server_timeout>, name, difficulty, ip, "<game>") {}
};
}
}
#endif'''

    adapt(header, include_path + '/game.h', description, False, True)
    
    # generate <game>_ai.h
    header = '''#ifndef <GAME>_AI_H
#define <GAME>_AI_H

#include <duels/player.h>
#include <duels/<game>/msg.h>

namespace duels {
namespace <game> {

// built-in AI class, should be heavily adapted to your needs
class <Game>AI : public duels::Player<inputMsg, feedbackMsg>
{
public:
  <Game>AI(int difficulty = 1) : difficulty(difficulty) {}

  void computeInput()
  {
    // in this function the `feedback` member variable that comes from the game
    // TODO update the `input` member variable
    // the `difficulty` member variable that may be used to tune your AI (0 = most stupidest)

  }

private:
  int difficulty = 1;
};
}
}
#endif
'''
    adapt(header, include_path + '/{}_ai.h'.format(game), description, False, False)
    
    # generate mechanics.h
    if description['turn_based']:
        header = '''#ifndef <GAME>_MECHANICS_H
#define <GAME>_MECHANICS_H

#include <duels/<game>/msg.h>

using namespace duels::<game>;

// base mechanics class, should be heavily adapted to reflect the game rules
class <Game>Mechanics
{
public:
    <Game>Mechanics() {}
    initMsg initGame() {return {};}
    void buildPlayerFeedback(feedbackMsg &feedback, [[maybe_unused]] bool player_1_turn)
    {

    }

private:

};

#endif 
'''
    else:
        header = '''#ifndef <GAME>_MECHANICS_H
#define <GAME>_MECHANICS_H

#include <duels/<game>/msg.h>

using namespace duels::<game>;

// base mechanics class, should be heavily adapted to reflect the game rules
class <Game>Mechanics
{
public:
    <Game>Mechanics() {}
    initMsg initGame() {return {};}
    void buildPlayerFeedbacks(feedbackMsg &feedback1, feedbackMsg &feedback2)
    {

    }

private:

};

#endif 
'''
    adapt(header, include_path + '/mechanics.h', description, False, False)


if __name__ == '__main__':
    
    game_path = len(sys.argv) == 2 and sys.argv[1] or '.'
    game_path = os.path.abspath(game_path) + '/'
    
    # installation from this file path
    duels_path = os.path.abspath(os.path.dirname(__file__) + '/..') + '/'

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
    description['game'] = game.lower()
    description['Game'] = game.title()
    description['GAME'] = game.upper()
    description['duels_path'] = duels_path[:-1]
    
    # erase times if detected in source
    if os.path.exists(game_path + 'server.cpp'):
        check_for = ['Timeout', 'Refresh']
        for line in read_file(game_path + 'server.cpp'):
            if len(check_for) == 0:
                break
            
            for key in check_for:
                idx = line.find(key)
                if idx > abs(line.find('//')):                    
                    value = line[idx+len(key):].split(')')
                    try:
                        value = int(''.join(c for c in value[0] if c.isdigit()))
                        if value != description[key.lower()]:
                            print('{}: set to {} ms in config but is {} ms in server.cpp'.format(key, description[key.lower()], value))
                            description[key.lower()] = value
                        check_for.remove(key)
                    except:
                        pass
    if 'server_timeout' not in description:
        description['server_timeout'] = 2*(description['timeout'] + description['refresh'])
                        

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
            
        adapt(duels_path + 'templates/server/' + src, dst_path, description, overwrite=False)
                
    # copy client templates
    for src,dst in (('CMakeLists.txt','CMakeLists.txt'), ('game.cpp', game+'.cpp')):
        dst_path = game_path + 'client_template/' + dst
        adapt(duels_path + 'templates/client/' + src, dst_path, description, overwrite=False)
