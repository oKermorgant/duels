#!/usr/bin/env python3
import os
import time
from threading import Thread
from pylab import *
import sys

class Msg:
    None
    
msg = Msg()

class Listener:
    def __init__(self, timeout):
        self.buff = '/tmp/cppgame'
        self.run = True
        self.data = ''
        self.prev = ''
        self.t = 0
        self.timeout = timeout
        self.ok = False
        
        self.axis = [0,1,0,1]
        
        if os.path.exists(self.buff):
            os.remove(self.buff)
        os.mkfifo(self.buff)        
        
        self.read_thr = Thread(target = self.loop)
        self.read_thr.start()
        
    def loop(self):
        while self.run:
            with open(self.buff) as f:
                self.data = f.read()
            print('Reading ' + self.data)
            
            # special cases
            if self.data == 'stop':
                self.run = False  
                break
            elif self.data != self.prev:
                self.prev = self.data
                self.t = time.time()
                print('Reading ' + self.data)
                
            if ' ' in self.data:
                self.ok = True
            
            
    def update_lim(self, x, y):
        self.axis[0] = min(x, self.axis[0])
        self.axis[1] = max(x, self.axis[1])
        self.axis[2] = min(y, self.axis[2])
        self.axis[3] = max(y, self.axis[3])
    
    def msg(self):
        global msg
        vals = self.data.strip().split()
        for i in range(0, len(vals), 2):
            setattr(msg, vals[i], float(vals[i+1]))
        return msg
    
    def timeout(self):
        return time.time() - self.t > self.timeout
                
listener = Listener(10)

close('all')
figure()
ax = gca()

pos, = ax.plot([], [], 'bo')               

tight_layout()
ion()
show(block=False)

while listener.run:
    if listener.ok:
        
        print('dt = {}'.format(time.time() - listener.t))
        if listener.timeout():
            listener.run = False
        
        listener.update_lim(msg.x1, msg.y1)    
        ax.axis(listener.axis)
        pos.set_data([msg.x1], [msg.y1])
        draw()
 
    pause(0.01)
    
close('all')
print('After loop')