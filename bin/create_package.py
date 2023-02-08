#!/usr/bin/python3

import os
from subprocess import run, check_output
import shutil
import sys
import filecmp

pjoin = os.path.join
version = '1.1'
duels_path = os.path.abspath(os.path.dirname(__file__) + '/..')
dest = '/opt/duels'
install = '-i' in sys.argv
clean_build = '-b' in sys.argv

for i,arg in enumerate(sys.argv):
    if arg == '-d':
        dest = sys.argv[i+1]
    
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
                if f not in ['CMakeLists.txt.user', 'client.h']:
                    f_mtime = os.stat(pjoin(directory,f)).st_mtime
                    if f_mtime > mtime:
                        mtime = f_mtime
                        latest_file = f
    return mtime,latest_file

class Game:
    
    OK = 0
    NOT_INSTALLED = 1
    NEED_RECOMPILE = 2
    NOT_SURE = 3
    NEED_REINSTALL = 4
    
    msg = {}
    msg[OK] = 'ready to ship'
    msg[NOT_INSTALLED] = 'not installed'
    msg[NEED_RECOMPILE] = 'was compiled before latest changes in duels library'
    msg[NOT_SURE] = 'not checked'
    msg[NEED_REINSTALL] = 'installed version is not last one'

    def __init__(self,directory):
        self.src = directory
        self.name = directory.split('/')[-1]
        self.binary = self.name + '_server'
        self.server = pjoin(directory, 'server.cpp')
        self.status = self.NOT_SURE
        self.installed_files = self.files()
    
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
            return f.read().splitlines()
    
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
            if os.path.exists(f'{duels_path}/include/duels/{self.name}'):
                self.status = self.NEED_REINSTALL
            else:
                self.status = self.NOT_INSTALLED
                
    def check_mtime(self):
        
        src = latest_mtime(self.src)
        build = latest_mtime(pjoin(self.src,'build'))
        
        # binary should be more recent than source
        if src[0] > build[0]:
            self.status = self.NEED_RECOMPILE
            self.latest = src[1]
            return
                
        # also more recent than duels
        server_stats = os.stat(pjoin(self.src,'build',self.binary))
        if server_stats.st_mtime < self.duels_mtime:
            self.latest = self.binary
            self.status = self.NEED_RECOMPILE     
            return
            
    def check_installed_version(self):
        
        to_check = dict([(os.path.basename(f), f) for f in self.installed_files])
        to_check.pop('CMakeLists.txt')
        self.latest = None
        
        # look for all installed files in the game dev folder
        for directory, subdirs, files in os.walk(self.src):            
            
            if '.git' in subdirs:
                subdirs.pop('.git')
                
            for f in list(to_check.keys()):
                if f in files:
                    if filecmp.cmp(pjoin(directory, f), to_check[f]):
                        to_check.pop(f)
                    else:
                        self.latest = f
            if len(to_check) == 0:
                break
            
        if len(to_check):
            print('   could not find following dev files for {}: {}'.format(self.name, ', '.join(to_check.keys())))
            
        if self.latest:
            self.status = self.NEED_REINSTALL
            
    def info(self, info):
        if info in (self.NEED_RECOMPILE, self.NEED_REINSTALL):
            return '{} ({})'.format(self.name, self.latest)
        return self.name
        
    def get_status(self):
        for f in (self.check_installed, self.check_mtime, self.check_installed_version):
            if self.status == self.NOT_SURE:
                f()
            else:
                break
            
        if self.status == self.NOT_SURE:
            self.status = self.OK
            
        if self.status in (self.NEED_RECOMPILE,self.NEED_REINSTALL) or (self.status==self.OK and clean_build):
            # reinstall
            add = ' (will recompile from scratch)' if clean_build else ''
            res = input(self.name + f': latest version does not seem to be installed. Install{add}? [Y/n] ')
            if res not in ('n','N'):
                build_dir = pjoin(self.src,'build')

                if clean_build:
                    shutil.rmtree(build_dir)
                    os.mkdir(build_dir)
                    check_output(['cmake','..',f'-DDUELS_ROOT={duels_path}'], cwd=build_dir)

                print(f'Compiling {self.name}...')
                check_output(['cmake', '--build', '.', '--target', 'install'], cwd=build_dir)
                self.status = self.OK
   
    def update_client(self):
        print('Updating client template for ' + self.name)
        client_cmake = pjoin(duels_path, 'games', self.name, 'CMakeLists.txt')
        with open(client_cmake) as f:
            cmake = f.read().splitlines()
        cmake_out = []
        for line in cmake:
            if '..' in line or 'will be' in line:
                continue
            elif '(DUELS_ROOT' in line:
                cmake_out.append(f'SET(DUELS_ROOT "{dest}" CACHE STRING "Path to duels installation folder")')
            else:
                cmake_out.append(line)
        with open(client_cmake,'w') as f:
            f.write('\n'.join(cmake_out))

# check potential sources
games = []
for directory,subdirs,files in os.walk(duels_path + '/..'):
    if directory.endswith('deb') or directory.endswith('examples'):
        subdirs.clear()
        continue
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
to_remove = games[Game.NEED_RECOMPILE]
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
if new_version != '':
    version = new_version

if os.path.exists(deb_root):
    run(['sudo','rm','-rf',deb_root])
os.makedirs(pkg_root)
os.makedirs(pjoin(deb_root, 'DEBIAN'))

for directory in ('bin','games','include','templates','examples'):
    shutil.copytree(pjoin(duels_path, directory), pjoin(pkg_root, directory))
    
size = check_output(['du', '-s', '--block-size=1024', deb_root])
print('Creating package from ' + duels_path)

control = f'''Package: duels
Version: {version}
Section: Education
Priority: optional
Architecture: all
Essential: no
Installed-Size: {size.decode('utf-8').split()[0]}
Depends: python3-pygame, libzmq3-dev, python3-zmq, libyaml-cpp-dev, python3-yaml
Maintainer: olivier.kermorgant@ec-nantes.fr
Description: The Duels package to practice game AI's
'''

with open(control_file, 'w') as f:
    f.write(control)

run(['sudo','chown', 'root:root', '.', '-R'], cwd = deb_root)
run(['sudo','chmod', 'a+rX', '.', '-R'], cwd = pkg_root)
run(['dpkg-deb', '--build', 'duels'], cwd = pjoin(duels_path, 'deb'))

distro = check_output(['lsb_release','-sc']).decode().strip()
versionned_name = pjoin(duels_path, 'deb', f'duels[{distro}]_{version}.deb'.format())
shutil.move(pjoin(duels_path, 'deb', 'duels.deb'), versionned_name)

if install:
    print('Installing package...')
    run(['sudo', 'dpkg', '-i', versionned_name])
