import time
import zmq
import sys
import yaml

class dict_to_obj(object):
  def __init__(self, adict):
    self.__dict__.update(adict)

class Subscriber:
    def __init__(self):
        self.ip = len(sys.argv) > 1 and sys.argv[1] or '127.0.0.1'
        self.port = len(sys.argv) > 2 and int(sys.argv[2]) or 3003
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.setsockopt( zmq.SUBSCRIBE, "" )
        self.socket.connect('tcp://{}:{}'.format(self.ip, self.port + 2 - self.port % 5))
        self.poller = zmq.Poller()
        self.poller.register(self.socket, zmq.POLLIN)
        print('Listening @ tcp://{}:{}'.format(self.ip, self.port  + 2 - self.port % 5))

        
    def get_init(self):        
        self.shake = self.context.socket(zmq.REP)
        print('shake @ tcp://{}:{}'.format(self.ip, self.port))
        self.shake.connect('tcp://{}:{}'.format(self.ip, self.port))
        #  Wait for next request from client
        init_msg = self.shake.recv()
        print("Received request: %s" % init_msg)
        return dict_to_obj(yaml.load(init_msg))
    
    def ready(self):
        self.shake.send(b'ok')
        self.shake.close()
        time.sleep(.1)
        
    def refresh(self):
        if not len(self.poller.poll(2000)):
            return dict_to_obj({'winner': -1})
        return dict_to_obj(yaml.load(self.socket.recv()))
    
