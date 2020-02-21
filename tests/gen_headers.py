#!/usr/bin/python3
import sys
import yaml
import os

def msg_derived(name, keys, field):
    
    l = len(keys)
    detail = [key.split() for key in keys]
    ret = 'struct {}\n{{\n'.format(name)
    ret += '  {};\n'.format('; '.join(keys))
    if field == 'display':
        ret += '  std::string toString()\n  {\n'
        ret += '    std::stringstream ss;\n'
        ret += '    ss << "{key} " << {key}\n'.format(key=detail[0][1])
        for T,key in detail[1:]:
            ret += '       << " {} " << {}\n'.format(key, key)
        ret = ret[:-1] + ';\n    return ss.str();\n  }\n'
    elif field == 'feedback':
        ret += '  {}() {{}}\n'.format(name)
        ret += '  {}({})\n'.format(name, ', '.join(['{} _{}'.format(T, key) for T,key in detail]))
        ret += '    : {} {{}}\n'.format(', '.join(['{key}(_{key})'.format(key=key) for T,key in detail]))
        ret += '''private:
  Status status = ONGOING;
  template <class T, class U, class V, int timeout>
  friend class GameServer;
  template <class T, class U, class V>
  friend class PlayerClient;\n'''
    return ret + '};'

def build_headers(msg_path, msg):
    with open(msg_path + msg) as f:
        content = yaml.safe_load(f)
    
    if 'name' in content:
        game = content['name']
        content.pop('name')
    else:
        game = msg.split('.')[0]
    guard = game.upper() + '_MSG_H'
    game = game.title()
    
    header = ['#ifndef {}'.format(guard)]
    header.append('#define {}'.format(guard))
    for include in ('game_server.h', 'player_client.h', 'sstream'):
        header.append('#include <{}>'.format(include))
    header.append('namespace ecn {{\nnamespace {} {{'.format(game.lower()))
    
    names = []
    for field in ('input', 'feedback', 'display'):
        names.append(field.title()+'Msg')
        header.append(msg_derived(names[-1], content[field], field))
        
    # TODO add path to display for client
    # probably add various other pathes...
    header.append('using Server = GameServer<{}, {}>;'.format(', '.join(names), content['timeout']))
    header.append('using Client = PlayerClient<{}>;'.format(', '.join(names)))
        
    header.append('}\n}\n#endif')
    with open(msg_path + msg.replace('.yaml', '.h'), 'w') as f:
        f.write('\n'.join(header))

msg_path = os.path.dirname(__file__) + '/games/'

for msg in os.listdir(msg_path):
    if msg.endswith('.yaml'):
        build_headers(msg_path, msg)
