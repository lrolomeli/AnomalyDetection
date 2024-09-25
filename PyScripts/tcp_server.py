import socket
import struct
import signal
import sys

# Define the server address and port
HOST = '192.168.100.248'  # Listen on all available interfaces
PORT = 5001       # Port to listen on
filename = 'rfile.csv'
BLOCK_SIZE = 2048  # We're expecting 2048 bytes
server_socket = None  # Global reference to the server socket
gdata = list()

def signal_handler(sig, frame):
    """Handle Ctrl+C (SIGINT) signal to safely unbind the socket and exit."""
    print("\nCaught Ctrl+C, shutting down the server...")
    if server_socket:
        server_socket.close()
        print("Server socket closed.")
    sys.exit(0)

# Function to ensure that we receive the full buffer
def receive_full_block(conn):
    data_buffer = bytearray()  # Temporary buffer to accumulate data
    total_received = 0

    while total_received < BLOCK_SIZE:
        chunk = conn.recv(BLOCK_SIZE - total_received)  # Receive the remaining bytes
        if not chunk:
            raise ConnectionError("Connection closed before receiving full block")

        data_buffer.extend(chunk)
        total_received += len(chunk)

        print(f"Received {len(chunk)} bytes, total received: {total_received}/{BLOCK_SIZE}")

    # Full block received, send ACK
    conn.sendall(b'OK')
    print(f"ACK sent after receiving full block of {total_received} bytes.")

    return data_buffer

def store_data(data):
    global gdata
    gdata.append(struct.unpack('<1024H', data))

def save_data():
    global gdata
    cnt = 1
    print("Storing data in csv file...")
    with open(filename, 'a') as file:
        for d in gdata:
            for v in d:
                file.write('' + str(cnt) + ',' + str(v) + '\n')
                cnt += 1
    print("File saved...")

def start_server():
    global server_socket
    global full_data
    global gdata
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind((HOST, PORT))
        server_socket.listen()
        print(f"Server listening on {HOST}:{PORT}")

        print("Waiting for a connection...")
        conn, addr = server_socket.accept()  # Accept a new connection
        with conn:
            print(f"Connected to {addr}")
            try:
                for i in range(940):
                    full_data = receive_full_block(conn)
                    store_data(full_data)
                    print(full_data[0])
                save_data()
            except ConnectionError as e:
                print(f"Error: {e}")
    except KeyboardInterrupt:
        print("\nServer interrupted by Ctrl+C, cleaning up...")
    finally:
        if server_socket:
            server_socket.close()  # Ensure the socket is closed
            print("Server socket closed. Exiting program.")


if __name__ == "__main__":
    # Set up signal handler for graceful shutdown on Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)
    
    start_server()