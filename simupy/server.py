import socket 
import struct
from os import path

recv_size = 1
send_size = 1
BYTE_SIZE = 8

# read port number
with open(path.join(path.dirname(__file__), "port.txt")) as f:
    port = int(next(f))
    print(port)

# establish socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('127.0.0.1', port))
s.listen(5)

# wait for client to connect
conn, addr = s.accept()
print('Connection address:', addr)

# echo data until no more data is received
while True:
    recv_data = conn.recv(recv_size*BYTE_SIZE)
    if not recv_data:
        break
    recv_data = struct.unpack(str(recv_size) + 'd', recv_data)
    print(recv_data)
    
    send_data = struct.pack(str(send_size)+ 'd', *recv_data)
    conn.sendall(send_data)  

conn.close()
