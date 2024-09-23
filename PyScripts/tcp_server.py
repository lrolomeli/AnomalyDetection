import socket
import struct

# Define the server address and port
HOST = '192.168.100.248'  # Listen on all available interfaces
PORT = 5001       # Port to listen on
file_path = 'example.txt'
rx_size = 2048  # We're expecting 2048 bytes

# Function to ensure that we receive the full buffer
def receive_all(sock):
    buffer = b''
    while len(buffer) < rx_size:
        chunk = sock.recv(rx_size - len(buffer))  # Receive the remaining bytes
        #print(chunk)
        if not chunk:
            raise ConnectionError("Socket connection broken")
        buffer += chunk
    return buffer

# Create a TCP socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((HOST, PORT))  # Bind the socket to the address and port
    server_socket.listen()            # Enable the server to accept connections

    print(f"Server listening on {HOST}:{PORT}")

    # Wait for a client to connect
    conn, addr = server_socket.accept()

    for i in range(4):
        buffer = receive_all(conn)
        # Use the receive_all function to ensure we get the complete 2048-byte buffer
        data = struct.unpack('<1024H', buffer)
        print(data)
        # Send back ACK
        conn.sendall(b'OK')  # Send an acknowledgment
        #with open(file_path, 'a') as file:
            #file.write(str(data) + '\n')  # Adds a newline after each entry
