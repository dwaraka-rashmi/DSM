# IMPORTS
from threading import Lock, Thread
import socket

# CONSTANTS
MONITOR_IP = "0.0.0.0"
MAX_CONN = 10  # Change to number of clients

# DATA DEFINITIONS


class PageTableEntry(object):
    def __init__(self):
        self.lock = Lock()
        self.clients = []


class Monitor(object):
    def __init__(self, port):
        self.port = port
        # key, val = client, socket
        self.clients = dict()
        self.monitorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def listen(self):
        self.monitorSocket.bind(("0.0.0.0", self.port))
        self.monitorSocket.listen(MAX_CONN)

        # Start a new thread for handling each client request
        while True:
            (client_socket, client_sock_address) = self.monitorSocket.accept()
            client_thread = Thread(
                target=self.clientHandler, args=client_socket)
            client_ip_addr = client_socket.getpeername()
            self.addClient(client_ip_addr, client_socket)
            client_thread.start()

    def addClient(self, client_ip_addr, client_socket):
        self.clients[client_ip_addr] = client_socket

    def clientHandler(self, client_sock):
        client_ip_addr = client_sock.getpeername()
        while True:
            try:
                data = None
                # get data from the client socket
            except:
                break
            process_data_thread = Thread(
                target=self.processData, args=data)
            process_data_thread.start()
        client_sock.close()

    def processData(self, data):
        pass  # Process data
