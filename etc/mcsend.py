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
    if(s == None):
        print('connecing')
        s = socket.socket()
        s.settimeout(2)
        s.connect((host, port))
        print('connected')
    else:
        print('already connected')

# Internal, send data buffer
def send(msg, size):
    global s
    if(s != None):
        totalsent = 0
        while totalsent < size:
            try:
                sent = s.send(msg[totalsent:])
                if sent == 0:
                    raise RuntimeError("socket connection broken")
                totalsent = totalsent + sent
            except Exception:
                s.close()
                s = None
                return

# Send a script command        
def sendCommand(command):
    send('scmd', 4)
    l32bit = struct.pack('i', len(command))
    send(l32bit, 4)
    send(command, len(command))

# Close the connection    
def bye():
    global s
    if(s != None):
        send("bye!", 4)
        l32bit = struct.pack('i', 0)
        send(l32bit, 4)
        print('sent bye waiting for world to end')
        while True:
            v = s.recv(20)
            print('recv')
            if not v:
                s.close()
                s = None
                return
