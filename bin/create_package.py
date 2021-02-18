#!/usr/bin/python3

import os
from subprocess import run, check_output
import shutil
import sys

pjoin = os.path.join
version = '1.1'
duels_path = os.path.abspath(os.path.dirname(__file__) + '/..')
dest = '/opt/duels'
install = False

for i,arg in enumerate(sys.argv):
    if arg == '-d':
        dest = sys.argv[i+1]
    elif arg == '-i':
        install = True
    
def latest_mtime(d, ignores = []):
    mtime = 0
    latest_file = ''
    ignores += ['build','.git']
    for directory,subdir,files in os.walk(d):
        i = 0
        while i < len(subdir):
            if subdir[i] in ignores:
                subdir.pop(i)
            else:
                i += 1
        if len(files):
            for f in files:
                if f not in ['CMakeLists.txt.user']:
                    f_mtime = os.stat(pjoin(directory,f)).st_mtime
                    if f_mtime > mtime:
                        mtime = f_mtime
                        latest_file = f
    return mtime,latest_file

class Game:
    
    OK = 0
    NOT_INSTALLED = 1
    LOCAL_FLAG = 2
    NEED_RECOMPILE = 3
    NOT_SURE = 4
    
    msg = {}
    msg[OK] = 'ready to ship'
    msg[NOT_INSTALLED] = 'not installed'
    msg[LOCAL_FLAG] = 'source has #define LOCAL_GAME'
    msg[NEED_RECOMPILE] = 'installed version less recent than duels library'
    msg[NOT_SURE] = 'not checked'

    def __init__(self,directory):
        self.src = directory
        self.name = directory.split('/')[-1]
        self.binary = self.name + '_server'
        self.server = pjoin(directory, 'server.cpp')
        self.status = self.NOT_SURE
    
    @staticmethod
    def register_duels_mtime(games):
        Game.duels_mtime,filename = latest_mtime(pjoin(duels_path, 'include'), [game.name for game in games])
        Game.msg[Game.NEED_RECOMPILE] += ' (' + filename + ')'
        
    def valid_source(self):
        return os.path.exists(pjoin(self.src, self.name + '.yaml')) and os.path.exists(self.server)
    
    def __str__(self):
        return self.name
    
    def files(self):
        install_manifest = pjoin(self.src, 'build','install_manifest.txt')
        if not os.path.exists(install_manifest):
            return None
        with open(install_manifest) as f:
            files = f.read().splitlines()
        return files
    
    def clean(self):
        print('Cleaning installed files for ' + self.name)
        files = self.files()
        for f in files:
            if os.path.exists(f):
                os.remove(f)
                base_dir = os.path.dirname(f)
                if os.path.split(base_dir)[-1] == self.name:
                    shutil.rmtree(base_dir)
    
    def check_installed(self):
        files = self.files()
        if files is None or not all(os.path.exists(f) for f in files):
            self.status = self.NOT_INSTALLED
    
    def check_local_flag(self):
        with open(self.server) as f:
            cpp = f.read().splitlines()
        local_game = False
        for line in cpp:
            if '#define LOCAL_GAME' in line and not(0 <= line.find('//') < line.find('#define LOCAL_GAME')):
                self.status = self.LOCAL_FLAG
                
    def check_mtime(self):
        
        src = latest_mtime(self.src)
        build = latest_mtime(pjoin(self.src,'build'))
        
        # binary should be more recent than source
        if src[0] > build[0]:
            self.status = self.NEED_RECOMPILE
            self.latest = src[1]
            return
        
        self.latest = self.binary
        # also more recent than duels
        server_stats = os.stat(pjoin(self.src,'build',self.binary))
        if server_stats.st_mtime < self.duels_mtime:
            self.status = self.NEED_RECOMPILE            
            return
        
        # also installed version should be the same
        self.status = self.OK
        if server_stats.st_size != os.stat(pjoin(duels_path,'bin',self.binary)).st_size:
            # reinstall
            res = input(self.name + ': latest version does not seem to be installed. Install? [Y/n] ')
            if res not in ('n','N'):
                check_output(['make', 'install'], cwd=pjoin(self.src,'build'))
                return                
            self.status = self.NEED_RECOMPILE
            
    def info(self, info):
        if info == self.NEED_RECOMPILE:
            return '{} ({})'.format(self.name, self.latest)
        return self.name
        
  
    def get_status(self):
        for f in (self.check_installed, self.check_local_flag, self.check_mtime):
            if self.status == self.NOT_SURE:
                f()
            else:
                break
   
    def update_client(self):
        print('Updating ' + self.name)
        client_cmake = pjoin(duels_path, 'games', self.name, 'CMakeLists.txt')
        with open(client_cmake) as f:
            cmake = f.read().splitlines()
        cmake_out = []
        for line in cmake:
            if '..' in line or 'will be' in line:
                continue
            elif duels_path in line:
                cmake_out.append(line.replace(duels_path, dest))
            else:
                cmake_out.append(line)
        with open(client_cmake,'w') as f:
            f.write('\n'.join(cmake_out))

# check potential sources
games = []
for directory,subdirs,files in os.walk(duels_path + '/..'):
    game = Game(directory)
    if game.valid_source():
        games.append(game)
        subdirs.clear()
    
# get last global include modification time
Game.register_duels_mtime(games)

for game in games:
    game.get_status()
    
# cluster according to status
games = dict((status, [game for game in games if game.status == status]) for status in Game.msg)

# print summary
for status in Game.msg:
    if len(games[status]):
        print('- {}: {}'.format(Game.msg[status], ', '.join(game.info(status) for game in games[status])))
print()
if len(games[Game.OK]):
    print('Will ship with {} '.format(', '.join(game.name for game in games[Game.OK])))
else:
    print('No games to ship')
    
print()
to_remove = games[Game.LOCAL_FLAG] + games[Game.NEED_RECOMPILE]
if len(to_remove):
    r = input('Uninstall old games and create package? [Y/n] ')
    if r in ('n','N'):
        sys.exit(0)
    else:
        for game in to_remove:
            game.clean()
else:
    r = input('Create package with current games? [Y/n] ')
    if r in ('n','N'):
        sys.exit(0)
            
# update valid games and create package
for game in games[Game.OK]:
    game.update_client()

deb_root = pjoin(duels_path, 'deb','duels')
pkg_root = pjoin(deb_root, dest[1:])
control_file = pjoin(deb_root, 'DEBIAN', 'control') 

if os.path.exists(control_file):
    with open(control_file) as f:
        version = f.read().splitlines()[1].strip().split()[-1]
    msg = 'Previous version: ' + version
else:
    msg = 'No previous version'
new_version = input(msg + ' / packaged version? [{}] '.format(version))
if new_version == '':
    new_version = version

if os.path.exists(deb_root):
    run(['sudo','rm','-rf',deb_root])
os.makedirs(pkg_root)
os.makedirs(pjoin(deb_root, 'DEBIAN'))

for directory in ('bin','games','include','templates'):
    shutil.copytree(pjoin(duels_path, directory), pjoin(pkg_root, directory))
    
size = check_output(['du', '-s', '--block-size=1024', deb_root])
print('Creating package from ' + duels_path)

control = '''Package: duels
Version: {}
Section: Education
Priority: optional
Architecture: all
Essential: no
Installed-Size: {}
Depends: python3-pygame, libzmq3-dev, python3-zmq
Maintainer: olivier.kermorgant@ec-nantes.fr
Description: The Duels package to practice game AI's
'''.format(version, size.decode('utf-8').split()[0])

with open(control_file, 'w') as f:
    f.write(control)

run(['sudo','chown', 'root:root', '.', '-R'], cwd = deb_root)
run(['dpkg-deb', '--build', 'duels'], cwd = pjoin(duels_path, 'deb'))

if install:
    print('Installing package...')
    run(['sudo', 'dpkg', '-i', pjoin(duels_path, 'deb', 'duels.deb')])
