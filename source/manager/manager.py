# IMPORTS
from threading import Lock, Thread
from constants import *
import socket
import os
import operator

class PageTableEntry:
    def __init__(self):
        self.lock = Lock()
        self.clients = []
        self.permission = NONE
        self.invalidations = {}
        self.encoded_page = "EXISTING"

class Manager:
    def __init__(self, port, pageCount):
        self.port = port
        self.clients = {}
        self.page_table_entries = [PageTableEntry()
                                   for i in xrange(pageCount)]
        self.managerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def Listen(self):
        self.managerSocket.bind((MANAGER_IP, self.port))
        self.managerSocket.listen(MAX_CONN)

        print "[Manager] Waiting for connections..."
        # Start a new thread for handling each client request
        while True:
            (client_socket, addr) = self.managerSocket.accept()
            print "[Manager] Accepted connection from client:", str(addr)
            client_thread = Thread(target=self.clientHandler, args=(client_socket, ))
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
                header_length = data.find(" ") + 1
                data = client_socket.recv(data_length + header_length, socket.MSG_WAITALL)
                if not data: break
            except: break
            process_data_thread = Thread(target = self.processData, args = (client, data.split(" ",1)[1]))
            process_data_thread.start()
        client_socket.close()


    def processData(self, client, data):
    	print data
        tokens = data.split(" ")
        if tokens[0] == "REQUESTPAGE":
            self.requestPage(client, int(tokens[2]) % NUMPAGES, tokens[1])
        elif tokens[0] == "INVALIDATE":
            encoded_page = tokens[3] if len(tokens) > 3 else ""
            self.invalidateConfirmation(client, int(tokens[2]) % NUMPAGES,encoded_page)
        else:
            print "Invalid message"

    def requestPage(self, client, page_number, permission):
        page_table_entry = self.page_table_entries[page_number]
        page_table_entry.lock.acquire()

        if page_table_entry.permission == NONE:
            page_table_entry.permission = permission
            page_table_entry.clients = [client]
            self.sendClientConfirmation(client, page_number, permission,
                                        page_table_entry.encoded_page)
            if permission == READ:
                page_table_entry.clients = [client]

            page_table_entry.lock.release()
            return

        if permission == WRITE:
            if page_table_entry.permission == WRITE:
                self.invalidate(client, page_number, True)
            else:
                self.invalidate(client, page_number, False)
            page_table_entry.clients = [client]
        

        if permission == READ:
            if page_table_entry.permission == WRITE:
            	self.invalidate(client, page_number, True)
                page_table_entry.clients = [client]
            else:
            	page_table_entry.clients.append(client)

        # print page_table_entry.encoded_page    	
        self.sendClientConfirmation(client, page_number, permission,page_table_entry.encoded_page)
        page_table_entry.permission = permission
        page_table_entry.lock.release()

    def sendClientConfirmation(self, client, page_number, permission, encoded_page):
    	message = "REQUESTPAGE " + permission + " CONFIRMATION " +str(page_number) + " " + str(encoded_page)
        print message
        # print "inside send confirmation"
        self.SendMsg(client, message)

    def SendMsg(self, client, message):
        client_socket = self.clients[client]
        message = str(len(message)) + " " + message
        # print "inside send message" , message
        client_socket.send(message)

    def invalidate(self, client, page_number, get_page):
        page_table_entry = self.page_table_entries[page_number]
        page_table_entry.invalidations = {}

        for user in page_table_entry.clients:
            if user != client:
                page_table_entry.invalidations[user] = False

        for user in page_table_entry.clients:
            if user != client:
                if get_page:
                    self.SendMsg(user, "INVALIDATE " + str(page_number) + " PAGEDATA")
                else:
                    self.SendMsg(user, "INVALIDATE " + str(page_number))

        while not reduce(operator.and_,page_table_entry.invalidations.values(),True):
            pass

    def invalidateConfirmation(self, client, page_number, data):
        page_table_entry = self.page_table_entries[page_number]
        page_table_entry.invalidations[client] = True

        if data:
        	# print data
        	page_table_entry.encoded_page = data


def main():
    try:
        manager = Manager(MANAGER_PORT, NUMPAGES)
        manager.Listen()
    except KeyboardInterrupt:
        for c, s in manager.clients.iteritems():
            s.close()
        os._exit(0)


if __name__ == '__main__':
    main()
