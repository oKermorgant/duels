#!/usr/bin/python
import sys
import os

project = 'pods'
if len(sys.argv) > 1:
    project = sys.argv[1]

base_path = os.path.dirname(__file__) + '/' + project + '/'
main = base_path + 'main.cpp'

class File:
    def __init__(self, name):
        self.name = name
        self.depends = []        
        self.valid = False
        
    def add_depend(self, name):
        self.depends.append(name)
        
    def rm_depend(self, name):
        if name in self.depends:
            self.depends.remove(name)
            
    def n_depends(self):
        return len(self.depends)    
    
    def __eq__(self, name):
        return self.name == name
    
    def display(self):
        print('{} -> {}'.format(self.name, self.depends))
        
    def __le__(self, other):
        return self.n_depends() < other.n_depends()
    
    def check(self, files):
        self.valid = True
        ret = self.depends[:]
        for f in files:
            if f.name in self.depends:
                ret += f.check(files)
        return ret
    
    def clean(self):
        with open(base_path + self.name) as io:
            content = io.read().splitlines()
        out = []
        includes = []
        for line in content:
            if '#include "' in line:
                continue
            elif '#include' in line:
                includes.append(line.strip())
                continue
            elif line.startswith('#') and '_H' in line:
                continue
            out.append(line)
        return '\n'.join(out), includes
        
# list files
files = [File('main.cpp')]
for f in os.listdir(base_path):
    if f.endswith('.h'):
        files.append(File(f))

# find hierarchy
for f in files:
    with open(base_path + f.name) as io:
        content = io.read()
    for f2 in files:
        if '"{}"'.format(f2.name) in content:
            f.add_depend(f2.name)

# test usefullness
files[0].check(files)
files = [f for f in files if f.valid]
main = files.pop(0)

order = []

while len(files):
    for f in files:
        if f.n_depends() == 0:
            order.append(f)
    files = [f for f in files if f.n_depends() != 0]
    for f in files:
        for fo in order:
            f.rm_depend(fo.name)

# build final codingame file + Python header
includes = []
header = ''
for f in order:
    h,i = f.clean()
    includes += i
    header += '\n' + h
    
core,i = main.clean()
includes += i

includes = '\n'.join(list(dict.fromkeys(includes)))

with open(base_path + 'glob_header.hpp', 'w') as f:
    f.write(includes + '\n' + header)
    
with open(base_path + 'codingame.cpp', 'w') as f:
    f.write(includes + '\n' + header + '\n' + core)    
