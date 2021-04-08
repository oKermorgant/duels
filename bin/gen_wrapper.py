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
        self.name = key
        if '(' in key:  # array / vector
            self.name, dim = key.split('(')
            dim = dim[:-1]
            print(key, self.name, dim)
            if dim:
                self.type = 'std::array<{}, {}>'.format(self.type, dim)
            else:
                self.type = 'std::vector<{}>'.format(self.type)
            
    def decl(self):
        return '{} {};'.format(self.type, self.name)
            
    def ss(self, nl):
        return 'ss << "{}{}: " << {};'.format(nl and '\\n' or '', self.name, self.name)
            
def build_print_fct(name, infos, more = []):
    ret = '  std::string {}({}) const \n  {{\n    std::stringstream ss;\n    '.format(name, ', '.join(more))
    for i,key in enumerate([Info(key) for key in more] + infos):
        if i:
            ret += '\n    '
        ret += key.ss(i)
    return ret + '\n    return ss.str();\n  }\n'


def build_util_struct(name, items, prefix = '', game=''):
    
    if any(' ' not in item for item in items): 
        # enum class, already have a ostream overload  
        return '{}enum class {}{{{}}};'.format(prefix, name, ','.join(items)), None
    
    # struct
    fields = [item.split()[1] for item in items]
    fields_eq = []
    for field in fields:
        fields_eq.append('{field} == other.{field}'.format(field=field))
    main = '''struct {Name}
{{
  {items};
  inline bool operator==(const {Name} &other) const
  {{
    return {fields_eq};
  }}
}};'''.format(Name=name.title(), items=';'.join(items), fields_eq=' && '.join(fields_eq))
  
    stream_data = " << ',';\n  ss << ".join('"{field}: " << {name}.{field}'.format(field=field, name=name.lower()) for field in fields);
    
    detail = '''inline std::ostream& operator<<(std::ostream& ss, const duels::{game}::{Name} &{name})
{{
  ss << "{{";
  ss << {stream_data} << "}}";
  return ss;
}}
'''.format(game=game.lower(), name=name.lower(), Name=name.title(), items=';'.join(items), stream_data = stream_data)
    
     
    return main, detail

def struct_name(field):
    return field.title().replace('_','')

def check_reserved_name(field, infos, reserved):
    names = [r.split()[-1] for r in reserved]
    for info in infos:
        if info.name in names:
            print('\033[93mCannot generate {}: `{}` cannot be used as a member variable because this name is used for internal logic\033[0m'.format(struct_name(field), info.name))
            sys.exit(1)

def core_msg_code(name, keys, field):
    
    ret = 'struct {}\n{{\n'.format(name)
    
    # build / eject enums
    infos = []
    for key in keys:
        if isinstance(key, dict):
            name,items = key.popitem()
            ret += build_util_struct(name, items, '  ')[0]
        else:
            infos.append(Info(key))
                
    if len(infos):
        ret += '  {}\n'.format(' '.join(info.decl() for info in infos))
        
    # check for reserved words
    toYAML = {'init_display': ('std::string name1', 'std::string name2') ,'display': ('Result result',)}
    
    if field in toYAML:
        check_reserved_name(field, infos, toYAML[field])
        ret += build_print_fct('toYAMLString', infos, toYAML[field])
    else:
        ret += build_print_fct('toString', infos)

    if field == 'feedback':
        state = 'State __state'
        check_reserved_name('feedback', infos, (state,))
        ret += '  {};\n'.format(state)
    return ret + '};\n'

msg_fields = ('init_display', 'input', 'feedback', 'display')

def build_headers(game, description, game_path):

    include_path = game_path + 'include/duels/'+game
    guard = game.upper() + '_MSG_H'

    # generate msg.h
    header = ['// generated from {}.yaml -- editing this file by hand is not recommended'.format(game.lower()),'#ifndef {}'.format(guard), '#define {}'.format(guard)]
    
    # generate msg_detail.h
    header_detail = ['// generated from {}.yaml -- editing this file by hand is not recommended'.format(game.lower())]
    
    includes = ('sstream', 'duels/game_state.h','duels/stream_overloads.h')
    
    header.append('\n'.join('#include <{}>'.format(inc) for inc in includes))
    header.append('namespace duels {{\nnamespace {} {{'.format(game.lower()))
                                                      
    if 'structs' in description:
        header.append('\n// utility structures')
        for name, items in description['structs'].items():
            main, detail = build_util_struct(name, items, game=game)            
            header.append(main)
            if detail:
                header_detail.append(detail)
        header.append('}}}}\n\n//detail on how to stream these structures\n#include <duels/{game}/msg_detail.h>\n\n// core game messages\nnamespace duels {{\nnamespace {game} {{'.format(game=game.lower()))
    
    names = []
    header.append('')
    for field in msg_fields:        
        names.append(struct_name(field))
        header.append(core_msg_code(names[-1], description[field], field))
        
    header.append('}}\n#endif')
    
    write_file(include_path + '/msg.h', header, overwrite=True)
    if 'structs' in description:
        write_file(include_path + '/msg_detail.h', header_detail, overwrite=True)
        
    # generate client.h
    header = '''#ifndef <GAME>_GAME_H
#define <GAME>_GAME_H
#include <duels/client.h>
#include <duels/<game>/msg.h>
#include <sstream>
namespace duels {
namespace <game> {
class Game: public duels::Client<Input, Feedback>
{
public:
  Game(int argc, char** argv, std::string name, int difficulty = 1)
    : Game(argc, argv, name, difficulty, "localhost") {}
  Game(int argc, char** argv, std::string name, std::string ip, int difficulty = 1)
      : Game(argc, argv, name, difficulty, ip) {}
private:
  Game(int argc, char** argv, std::string name, int difficulty, std::string ip)
      : duels::Client<Input, Feedback>(
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
class <Game>AI : public duels::Player<Input, Feedback>
{
public:
  <Game>AI(int difficulty = 1) : difficulty(difficulty) {}

  void updateInput()
  {
    // in this function the `feedback` member variable was updated from the game
    // TODO update the `input` member variable
    // the `difficulty` member variable may be used to tune your AI (0 = most stupidest)
    // do not hesitate to create a .cpp file if this function is long
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
    InitDisplay initGame() {return {};}
    inline const Display& display() const {return display_msg;}
    
    void buildPlayerFeedback(Feedback &feedback, [[maybe_unused]] bool player_1_turn)
    {

    }
    // TODO actually build / update display_msg from player input 
    

private:
  Display display_msg;
};

#endif 
'''
    else:
        header = '''#ifndef <GAME>_MECHANICS_H
#define <GAME>_MECHANICS_H

#include <duels/<game>/msg.h>

using namespace duels::<game>;
using duels::State;

// base mechanics class, should be heavily adapted to reflect the game rules
class <Game>Mechanics
{
public:
    <Game>Mechanics() {}
    InitDisplay initGame() {return {};}
    inline const Display& display() const {return display_msg;}
    
    // game evolution can be put here, or just save the inputs for later when building the feedbacks
    void update(const Input &input1, const Input &input2);
    
    // should return who has just won, if any. May also compute display
    Result buildPlayerFeedbacks(Feedback &feedback1, Feedback &feedback2)
    {
        return Result::NONE;    // game goes on
    }
    
    
    

private:
  Display display_msg;
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
        
    if game in ('algo', 'utils'):
        print('Your game cannot be called {} as this is a reserved include folder'.format(game))
        sys.exit(0)
        
    if description_file == '':
        print('Could not find any game description file in {}'.format(game_path))
        sys.exit(0)
        
    description = {'timeout': 100, 'turn_based': False, 'game': game, 'Game': game.title().replace('_',''), 'GAME': game.upper(), 
                'duels_path': duels_path[:-1], 'msg_detail': ''}
    for msg in msg_fields:
        description[msg] = []
    
    with open(description_file) as f:
        description.update(yaml.safe_load(f))
                
    if 'refresh' not in description:
        description['refresh'] = description['timeout']
    if 'structs' in description:
        description['msg_detail'] = 'include/duels/{}/msg_detail.h'.format(game)
    
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
