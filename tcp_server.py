import socket

HOST = '0.0.0.0'  # listen on all interfaces
PORT = 19008

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
print(f'Listening on {HOST}:{PORT} ...')
print('Waiting for MCU to connect...\n')

conn, addr = s.accept()
print(f'Connected from: {addr}\n')

try:
    while True:
        data = conn.recv(4096)
        if not data:
            break
        print(data.decode(), end='')
except KeyboardInterrupt:
    print('\n\nServer stopped.')
finally:
    conn.close()
    s.close()
