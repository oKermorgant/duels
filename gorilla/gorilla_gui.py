
import time
import zmq
import sys

context = zmq.Context()
shake = context.socket(zmq.REP)
shake.connect("tcp://127.0.0.1:300{}".format(sys.argv[1]))

#  Wait for next request from client
init_msg = shake.recv()
print("Received request: %s" % init_msg)

shake.send(b'ok')

shake.close()
time.sleep(.1)
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:3002")
socket.setsockopt( zmq.SUBSCRIBE, "" )
while True:
    display = socket.recv()
    print("Received msg: %s" % display)
