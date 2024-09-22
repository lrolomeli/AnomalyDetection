import socket
import struct

# Define the server address and port
HOST = '192.168.100.248'  # Listen on all available interfaces
PORT = 5001       # Port to listen on

file_path = 'example.txt'
rx_size = 2048  # We're expecting 2048 bytes

buffer = b''  # Start with an empty buffer

# Function to ensure that we receive the full buffer
def receive_all(sock, buffer_size):
    
    while len(buffer) < buffer_size:
        chunk = sock.recv(buffer_size - len(buffer))  # Receive the remaining bytes
        if not chunk:
            raise ConnectionError("Socket connection broken")
            return False
        buffer += chunk
    return True

# Create a TCP socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((HOST, PORT))  # Bind the socket to the address and port
    server_socket.listen()            # Enable the server to accept connections

    print(f"Server listening on {HOST}:{PORT}")

    # Wait for a client to connect
    conn, addr = server_socket.accept()
    i = 0
    
    while receive_all(conn, rx_size):
        # Use the receive_all function to ensure we get the complete 2048-byte buffer
        receive_all(conn, rx_size)
        data = struct.unpack('>1024H', buffer)
        print(i += 1)
        #with open(file_path, 'a') as file:
            #file.write(str(data) + '\n')  # Adds a newline after each entry
