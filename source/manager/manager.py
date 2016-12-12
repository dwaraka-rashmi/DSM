# IMPORTS
from threading import Lock, Thread
from constants import *
import socket
import os

# DATA DEFINITIONS

class PageTableEntry(object):
    def __init__(self):
        self.lock = Lock()
        self.clients = []
        self.permission = None
        self.invalidations = {}
        self.encoded_page = "EXISTING"


class Manager(object):
    def __init__(self, port, page_count):
        self.port = port
        # key, val = client, socket
        self.clients = dict()
        self.page_table_entries = [PageTableEntry()
                                   for i in xrange(page_count)]
        self.managerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def listen(self):
        self.managerSocket.bind((MANAGER_IP, self.port))
        self.managerSocket.listen(MAX_CONN)

        print "[Manager] Waiting for connections..."
        # Start a new thread for handling each client request
        while True:
            (client_socket, addr) = self.managerSocket.accept()
            print "[Manager] Accepted connection from client:", str(addr)
            client_thread = Thread(
                target=self.clientHandler, args=(client_socket, ))
            client = client_socket.getpeername()
            print "[Manager] Client ID:", str(client)
            self.addClient(client, client_socket)
            client_thread.start()

    def addClient(self, client, client_socket):
        self.clients[client] = client_socket

    def clientHandler(self, client_socket):
        client = client_socket.getpeername()
        while True:
            try:
                data = client_socket.recv(10, socket.MSG_PEEK)
                data_length = int(data.split(" ")[0])
                header_length = data.find(" ", 1)
                data = client_socket.recv(data_length + header_length,
                                          socket.MSG_WAITALL)
                if not data:
                    break
            except:
                break
            actual_data = data.split(" ", 1)[1]
            process_data_thread = Thread(
                target=self.processData, args=(client, actual_data))
            process_data_thread.start()
        client_socket.close()

    def processData(self, data):
        tokens = data.split(" ")
        if tokens[0] == "REQUEST":
            self.requestPage(client, int(args[2]) % NUMPAGES, args[1])
        elif tokens[0] == "INVALIDATE":
            encoded_page = args[3] if len(args) > 3 else ""
            self.invalidateConfirmation(client, encoded_page)
        else:
            print "Invalid message"

    def requestPage(self, client, page_number, permission):
        page_table_entry = self.page_table_entries[page_number]

        if page_table_entry.permission == NONE:
            page_table_entry.permission = permission
            page_table_entry.clients = [client]
            self.sendClientConfirmation(self, client, page_number, permission,
                                        page_table_entry.encoded_page)
            if permission == READ:
                page_table_entry.clients = [client]

            return

        if permission == WRITE:
            if page_table_entry.permission == WRITE:
                self.invalidate(client, page_number, True)
            else:
                self.invalidate(client, page_number, False)

        if permission == READ:
            if page_table_entry.permission == READ:
                page_table_entry.clients.append(client)
            else:
                self.invalidate(client, page_number, True)
                page_table_entry.users = [client]

        self.sendClientConfirmation(client, page_number, permission,
                                    page_table_entry.encoded_page)
        page_table_entry.permission = permission

    def sendClientConfirmation(
            self, client, page_number, permission, encoded_page):
        self.Send(client, "REQUESTPAGE " + permission + " CONFIRMATION " +
                  str(pagenumbers) + " " + str(encoded_page))

    def send(self, client, message):
        client_socket = self.clients[client]
        message = str(len(message)) + " " + message
        socket.send(message)

    def invalidate(self, client, page_number, get_page):
        page_table_entry = self.page_table_entries[client]
        page_table_entry.invalidations = dict()

        for user in page_table_entry.users:
            if user != client:
                page_table_entry.invalidate_confirmations[user] = False

        for user in page_table_entry.users:
            if user != client:
                if getpage:
                    self.Send(user, "INVALIDATE " +
                              str(pagenumber) + " PAGEDATA")
                else:
                    self.Send(user, "INVALIDATE " + str(pagenumber))

        while not reduce(operator.and_,
                         page_table_entry.invalidate_confirmations.values(),
                         True):
            pass

    def invalidateConfirmation(self, client, page_number, data):
        page_table_entry = self.page_table_entries[client]
        page_table_entry.invalidations[client] = False

        if data:
            page_table_entry.encoded_page = data


def main():
    try:
        manager = Manager(MANAGER_PORT, NUMPAGES)
        manager.listen()
    except KeyboardInterrupt:
        for c, s in manager.clients.iteritems():
            s.close()
        os._exit(0)


if __name__ == '__main__':
    main()
