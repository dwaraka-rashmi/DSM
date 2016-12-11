#ifndef _RPC_H
#define _RPC_H

#include <err.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int initSocket(char *ip, int port);
int destroySocket(void);
void *listener(void *ptr);
int messageHandler(char *payload);
int invalidate(char *payload);
void confirmInvalidateEncoded(int pgnum, char *encodedPage);
void confirmInvalidate(int pgnum);
int sendMessage(char *message);
int handlePageRequest(char *msg);
int request_page(int pgnum, char *type);

#endif