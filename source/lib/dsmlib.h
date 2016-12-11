#ifndef _DSM_LIB_H
#define _DSM_LIB_H

#include <signal.h>
#include <stdint.h>
#include <ucontext.h>
 
// Constants
#define MAX_SHARED_AREAS 100
#define MAX_SHARED_PAGES 1000000

// Data definitions
struct shared_area {
	uintptr_t start;
	size_t length;
}

// Functions
int dsmlib_init(char *ip, int port, uintptr_t starta, size_t len);
int dsmlib_destroy(void);
void page_fault_handler(int signum, siginfo_t *siginfo, ucontext_t *cont)

#endif