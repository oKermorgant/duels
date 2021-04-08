import time
import zmq
import sys
import yaml
import threading
import os

class dict_to_obj(object):
    def __init__(self, adict):
        self.__dict__.update(adict)
            
def to_object(msg):
    if isinstance(msg, list):
        return [to_object(v) for v in msg]
    elif isinstance(msg, dict):
        for v in msg:
            msg[v] = to_object(msg[v])
        return dict_to_obj(msg)
    return msg        
    
class Subscriber:
    def __init__(self, server_timeout = 2000):
        # sys.argv begins with path to this file
        self.ip = len(sys.argv) > 2 and sys.argv[2] or '127.0.0.1'
        self.port = len(sys.argv) > 3 and int(sys.argv[3]) or 3003
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.setsockopt_string( zmq.SUBSCRIBE, "" )
        self.socket.connect('tcp://{}:{}'.format(self.ip, self.port - (self.port % 5) + 2))
        self.server_timeout = server_timeout
        #print('display in on port {}'.format(self.port + 2 - self.port % 5))

        self.poller = zmq.Poller()
        self.poller.register(self.socket, zmq.POLLIN)
        
        # force pygame to quit if client has disappeared
        self.winner = 0
        self.reason = 'fair victory'

        if len(sys.argv) > 4:
            self.monitor = threading.Thread(target=self.check_client, args=(int(sys.argv[4]),))
            self.monitor.start()
        
    def check_client(self, client_pid):
        
        client_up = True
        while client_up and not self.winner:
            try:
                os.kill(client_pid, 0)
            except OSError:
                client_up = False
                break            
            time.sleep(0.1)  
        if not client_up:
            try:          
                pygame.display.quit()
                pygame.quit()
            except:
                pass
            sys.exit(0)
        
    def get_init(self, debug=False):        
        self.shake = self.context.socket(zmq.REP)
        self.shake.connect('tcp://{}:{}'.format(self.ip, self.port))
        #  Wait for next request from client
        msg = self.shake.recv()
        if debug:
            print(msg)
        return to_object(yaml.safe_load(msg))
    
    def ready(self):
        self.shake.send(b'ok')
        self.shake.close()
        time.sleep(.1)
        
    def refresh(self, debug=False):
        if self.winner:
            return None
        if not len(self.poller.poll(self.server_timeout)):
            self.winner = -1
            return None
        msg = self.socket.recv()
        if debug:
            print(msg)
        msg = to_object(yaml.safe_load(msg))
        self.winner = msg.result
        return msg
    
    def winner_name(self, init_msg):
        return getattr(init_msg, 'name{}'.format(self.winner))
    
