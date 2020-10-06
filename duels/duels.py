import time
import zmq
import sys
import yaml

class dict_to_obj(object):
  def __init__(self, adict):
    self.__dict__.update(adict)

class Subscriber:
    def __init__(self):
        # sys.argv begins with path to this file
        self.ip = len(sys.argv) > 2 and sys.argv[2] or '127.0.0.1'
        self.port = len(sys.argv) > 3 and int(sys.argv[3]) or 3003
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.setsockopt_string( zmq.SUBSCRIBE, "" )
        self.socket.connect('tcp://{}:{}'.format(self.ip, self.port + 2 - self.port % 5))
        self.poller = zmq.Poller()
        self.poller.register(self.socket, zmq.POLLIN)

        
    def get_init(self, debug=False):        
        self.shake = self.context.socket(zmq.REP)
        self.shake.connect('tcp://{}:{}'.format(self.ip, self.port))
        #  Wait for next request from client
        msg = self.shake.recv()
        if debug:
            print(yaml.safe_load(msg))
        return dict_to_obj(yaml.safe_load(msg))
    
    def ready(self):
        self.shake.send(b'ok')
        self.shake.close()
        time.sleep(.1)
        
    def refresh(self, debug=False):
        if not len(self.poller.poll(2000)):
            return dict_to_obj({'winner': -1})
        msg = self.socket.recv()
        if debug:
            print(yaml.safe_load(msg))
        return dict_to_obj(yaml.safe_load(msg))
    
