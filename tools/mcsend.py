import socket
import struct

s = socket.socket()

# Connect to a mission control server
def connect(host = 'localhost', port = 22500):
    s.connect((host, port))

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
    send("bye!", 4)
    l32bit = struct.pack('i', 0)
    send(l32bit, 4)
    while True:
        v = s.recv(20)
        if v == '':
            s.close()
            return
