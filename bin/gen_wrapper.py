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
    
    if os.path.exists(filename):
        if not overwrite:
            print('Skipping {}, file exists'.format(filename))
            return
        else:
            print('Overwriting {}'.format(filename))
    else:
        print('Creating {}'.format(filename))
    
    if not os.path.exists(os.path.dirname(filename)):
        os.mkdir(os.path.dirname(filename))
    with open(filename, 'w') as f:
        if isinstance(content, str):
            f.write(content)
        else:
            f.write('\n'.join(content))


def dict_replace(s, d):
    for key in d:
        s = s.replace('<'+key+'>', str(d[key]))
    return s


def adapt(str_in, file_out, description, file_in = True, overwrite = False):
    if file_in:
        write_file(file_out, dict_replace(read_file(str_in, False), description), overwrite)
    else:
        write_file(file_out, dict_replace(str_in, description), overwrite)


def_types = {'int': 0, 'float': 0, 'bool': False}


class Info:
    def __init__(self, key):
        self.type, key = key.split()
        self.name = key
        if '(' in key:  # array / vector
            self.name, dim = key.split('(')
            dim = dim[:-1]
            if dim:
                self.type = f'std::array<{self.type}, {dim}>'
            else:
                self.type = f'std::vector<{self.type}>'

    def py(self):
        if 'std::' in self.type or 'duels::' in self.type:
            return []
        else:
            return def_types[self.type]
            
    def decl(self):
        return '{} {};'.format(self.type, self.name)
            
    def ss(self, nl):
        return 'ss << "{}{}: " << {};'.format(nl and '\\n' or '', self.name, self.name)
    
    def node(self):
        return f'{self.name} = node["{self.name}"].as<{self.type}>();'
            

def serialize_fct(infos, more = []):
    ret = '  std::string serialize({}) const \n  {{\n    std::stringstream ss;\n    '.format(', '.join(more))
    for i,key in enumerate([Info(key) for key in more] + infos):
        if i:
            ret += '\n    '
        ret += key.ss(i)
    return ret + '\n    return ss.str();\n  }\n'


def deserialize_fct(infos):
    ret = '  void deserialize(const std::string &yaml)\n  {\n    const auto node{YAML::Load(yaml)};\n    '
    for i,key in enumerate(infos):
        if i:
            ret += '\n    '
        ret += key.node()
    return ret + '\n  }\n'


util_structs = []


def build_util_struct(name, items, prefix = '', game='', custom = []):

    def_types[name] = {}

    if any(' ' not in item for item in items):
        # enum class, already have a ostream overload
        def_types[name] = f'{name}.{items[0]}'
        return '{}enum class {}{{{}}};'.format(prefix, name, ','.join(items)), None, None

    ns = f'duels::{game}::'
    
    # struct, test for vectors / arrays
    for i,item in enumerate(items):
        if '(' in item:
            base,field,dim = item.replace('(',' ').split()
            if '::' not in base and base[0].isupper():
                base = f'{ns}{base}'
            dim = dim[:-1]

            if dim:
                items[i] = f'std::array<{base},{dim}> {field}'
            else:
                items[i] = f'std::vector<{base}> {field}'

    fields = [item.split()[1] for item in items]
    for item in items:
        field_type, field = item.split()
        if 'std::' in field_type or 'duels::' in field_type:
            def_types[name][field] = []
        else:
            def_types[name][field] = def_types[field_type]

    fields_eq = []
    for field in fields:
        fields_eq.append(f'{field} == other.{field}')

    main = '''struct {Name}
{{
  {items};
  inline bool operator==(const {Name} &other) const
  {{
    return {fields_eq};
  }}
}};'''.format(Name=name, items=';'.join(items), fields_eq=' && '.join(fields_eq))
  
    stream_data = " << ',';\n  ss << ".join('"{field}: " << {name}.{field}'.format(field=field, name=name.lower()) for field in fields);
    
    detail = '''inline std::ostream& operator<<(std::ostream& ss, const duels::{game}::{Name} &{name})
{{
  ss << "{{";
  ss << {stream_data} << "}}";
  return ss;
}}
'''.format(game=game.lower(), name=name.lower(), Name=name, items=';'.join(items), stream_data = stream_data)

    yaml_detail = []
    for item in items:
        t,attr = item.split()
        if t in custom:
            t = f'duels::{game}::{t}'
        elif t == 'bool':   # yaml expects 'true' or 'false' for Booleans: load as short
            t = 'short'
        yaml_detail.append(f'rhs.{attr} = node["{attr}"].as<{t}>();')
    yaml_detail = '\n    '.join(yaml_detail)

    yaml_detail = f'''template<>
struct convert<duels::{game}::{name}> \n{{
  static bool decode(Node const& node, duels::{game}::{name} & rhs)
  {{
    {yaml_detail}
    return true;
  }}
}};\n'''
    
    return main.replace(ns, ''), detail, yaml_detail


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
            
    if field == 'feedback':
        state = 'State __state'
        check_reserved_name('feedback', infos, (state,))
        infos.append(Info(state))

    fwd_py = name in ('InitDisplay', 'Display')

    if fwd_py:
        def_types[name] = {}
                
    if len(infos):
        ret += '  {}\n'.format(' '.join(info.decl() for info in infos))
        if fwd_py:
            for info in infos:
                def_types[name][info.name] = info.py()

    # check for reserved words
    toYAML = {'feedback': [], 'input': [], 'init_display': ('std::string name1', 'std::string name2'), 'display': ('Result result',)}
    
    check_reserved_name(field, infos, toYAML[field])
    ret += serialize_fct(infos, toYAML[field])
    
    if not toYAML[field]:
        ret += deserialize_fct(infos)

    return ret + '};\n'


def build_python_enums(enums, mod_file):

    if not len(enums):
        return []

    # get which enums are actually used in Python
    display_types = ' '.join(description['init_display'] + description['display'])
    change = 'structs' in description
    while change:
        change = False
        for s,des in description['structs'].items():
            if s in display_types and any([' ' in elem for elem in des]):
                change = True
                display_types = display_types.replace(s, str(des))

    py = []
    classes = []

    for enum in enums:
        enum = enum.replace('enum class', '').split('{')
        name = enum[0].strip()
        if name not in display_types:
            continue
        items = enum[1].replace('}', ',').split(',')[:-1]
        py.append(f'class {name}:')
        for i,item in enumerate(items):
            py.append(f'    {item.strip()} = {i}')
        py.append('')
        classes.append(name)

    if not len(classes):
        return

    description['enums_py'] = f'from {game}.enums import {",".join(classes)}'

    write_file(mod_file, py, overwrite=True)
    write_file(os.path.dirname(mod_file) + '/__init__.py', [], overwrite=False)


msg_fields = ('init_display', 'input', 'feedback', 'display')


def build_headers(game, description, game_path):

    include_path = game_path + 'include/duels/'+game
    guard = game.upper() + '_MSG_H'

    # generate msg.h
    header = [f'// generated from {game.lower()}.yaml -- editing this file by hand is not recommended',f'#ifndef {guard}', f'#define {guard}']
    
    # generate msg_detail.h
    header_detail = ['// generated from {}.yaml -- editing this file by hand is not recommended'.format(game.lower())]
    
    includes = ('sstream', 'duels/game_state.h')
    
    header.append('\n'.join(f'#include <{inc}>' for inc in includes))
    header.append(f'namespace duels {{\nnamespace {game.lower()} {{')
                                                      
    whole_yaml_detail = []

    if 'structs' in description:
        header.append('\n// utility structures')

        for name, items in description['structs'].items():
            main, detail, yaml_detail = build_util_struct(name, items, game=game, custom = list(description['structs'].keys()))
            header.append(main)
            if detail:
                header_detail.append(detail)
                whole_yaml_detail.append(yaml_detail)
        header.append('}}}}\n\n//detail on how to stream these structures\n#include "msg_detail.h"\n\n// core game messages\nnamespace duels {{\nnamespace {game} {{'.format(game=game.lower()))

    build_python_enums([line for line in header if 'enum class' in line], f'{game_path}/{game}/enums.py')

    names = []
    header.append('')
    for field in msg_fields:
        names.append(struct_name(field))
        header.append(core_msg_code(names[-1], description[field], field))

    # check for builtin messages
    builtin = set()
    for line in header:
        while 'duels::' in line:
            line = line.split('duels::',1)[1]
            end = [line.find(c) for c in ' ,>&']
            builtin.add(line[:min(e for e in end if e > 0)])

    for msg in builtin:
        header.insert(4, f'#include <duels/msg/{msg.lower()}.h>')

    header.append('}}\n#endif')
    
    write_file(include_path + '/msg.h', header, overwrite=True)
    if 'structs' in description:
        
        header_detail.append('namespace YAML\n{{\n{}}}'.format('\n'.join(whole_yaml_detail)))
        
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
  /// to play as player 1 against some AI
  Game(int argc, char** argv, std::string name, int difficulty)
    : Game(argc, argv, name, difficulty, "localhost") {}
  /// to play as player 2 against some AI
  Game(int argc, char** argv, int difficulty, std::string name)
    : Game(argc, argv, name, -difficulty, "localhost") {}
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

namespace duels {
namespace <game> {

// base mechanics class, should be heavily adapted to reflect the game rules
class Mechanics
{
public:
    Mechanics() {}
    InitDisplay initGame() {return {};}
    inline const Display& display() const {return display_msg;}
    
    void buildPlayerFeedback(Feedback &feedback, [[maybe_unused]] bool player_1_turn)
    {

    }
    // TODO actually build / update display_msg from player input
    

private:
  Display display_msg;
};
}
}
#endif
'''
    else:
        header = '''#ifndef <GAME>_MECHANICS_H
#define <GAME>_MECHANICS_H

#include <duels/<game>/msg.h>

namespace duels {
namespace <game> {

// base mechanics class, should be heavily adapted to reflect the game rules
class Mechanics
{
public:
    Mechanics() {}
    InitDisplay initGame() {return {};}
    inline const Display& display() const {return display_msg;}
    
    // game evolution can be put here, or just save the inputs for later when building the feedbacks
    void update(const Input &input1, const Input &input2) {}
    
    // should return who has just won, if any. May also compute display
    Result buildPlayerFeedbacks(Feedback &feedback1, Feedback &feedback2)
    {
        return Result::NONE;    // game goes on
    }

private:
  Display display_msg;
};
}
}

#endif
'''
    adapt(header, include_path + '/mechanics.h', description, False, False)

def adapt_enums(d):
    d = str(d)
    # change string enums to actual value
    for val in def_types.values():
        if type(val) == str and '.' in val:
            d = d.replace(f"'{val}'", val)
    return d

if __name__ == '__main__':
    
    game_path = len(sys.argv) == 2 and sys.argv[1] or '.'
    game_path = os.path.abspath(game_path)
    game = os.path.basename(game_path)
    game_path += '/'
    description_file = f'{game_path}{game}.yaml'
    
    # installation from this file path
    duels_path = os.path.abspath(os.path.dirname(__file__) + '/..') + '/'
        
    if game in ('algo', 'utils'):
        print(f'Your game cannot be called {game} as this is a reserved include folder')
        sys.exit(0)
        
    if description_file == '':
        print(f'Could not find any game description file in {game_path}')
        sys.exit(0)
        
    description = {'timeout': 100, 'turn_based': False, 'game': game, 'Game': game.title().replace('_',''), 'GAME': game.upper(), 
                'duels_path': duels_path[:-1], 'msg_detail': '', 'enums_py': ''}
    for msg in msg_fields:
        description[msg] = []
    
    with open(description_file) as f:
        description.update(yaml.safe_load(f))
                
    if 'refresh' not in description:
        description['refresh'] = description['timeout']
    if 'structs' in description:
        description['msg_detail'] = f'include/duels/{game}/msg_detail.h'
    
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

    description['init_msg_py'] = adapt_enums(def_types['InitDisplay'])
    description['msg_py'] = adapt_enums(def_types['Display'])
    
    # copy server templates
    for src in ('CMakeLists.txt', description['turn_based'] and 'server_turns.cpp' or 'server.cpp', 'gui.py'):
        
        dst_path = game_path + src
        
        if src == 'gui.py':
            dst_path = game_path + game + '_gui.py'
        elif src == 'server_turns.cpp':
            dst_path = game_path + 'server.cpp'
            
        adapt(duels_path + 'templates/server/' + src, dst_path, description, overwrite=False)
                
    # copy client templates
    for src,dst in (('CMakeLists.txt','CMakeLists.txt'), ('game.cpp', game+'.cpp')):
        dst_path = game_path + 'client_template/' + dst
        adapt(duels_path + 'templates/client/' + src, dst_path, description, overwrite=False)
