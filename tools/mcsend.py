# This module implements a basic omegalib MissionControl client in python
# usage example:
# connect([host])       < connects to an application running on [host]
# sendCommand('foo()')  < calls python function foo in the application
# bye()                 < closes the connection
import socket
import struct

s = None

# Connect to a mission control server
def connect(host = 'localhost', port = 22500):
    global s
    print('connecing')
    s = socket.socket()
    s.connect((host, port))
    print('connected')

# Internal, send data buffer
def send(msg, size):
    totalsent = 0
    while totalsent < size:
        sent = s.send(msg[totalsent:])
        if sent == 0:
            raise RuntimeError("socket connection broken")
        totalsent = totalsent + sent

# Send a script command        
def sendCommand(command):
    send('scmd', 4)
    l32bit = struct.pack('i', len(command))
    send(l32bit, 4)
    send(command, len(command))

# Close the connection    
def bye():
    global s
    send("bye!", 4)
    l32bit = struct.pack('i', 0)
    send(l32bit, 4)
    while True:
        v = s.recv(20)
        if v == '':
            s.close()
            s = None
            return
