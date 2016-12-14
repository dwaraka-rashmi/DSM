#ifndef _DSM_LIB_H
#define _DSM_LIB_H

#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
 
// Constants
#define MAX_SHARED_AREAS 100

// Data definitions
struct shared_area {
	uintptr_t start;
	size_t length;
};

// Functions
int dsmlib_init(char *ip, int port, uintptr_t start, size_t len);
int dsmlib_destroy(void);
void page_fault_handler(int signum, siginfo_t *siginfo, ucontext_t *cont);
int read_write_handler(int page_number,char* operation);
int add_shared_area(uintptr_t start, size_t len);

#endif