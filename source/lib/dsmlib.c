#include <signal.h>
#include <pthread.h>
#include <signal.h>

#include "b64.h"
#include "dsmlib.h"

// Old signal handler
static struct sigaction old_sig_action;

// Shared areas
int next_shared_page = 0;
struct shared_area shared_areas[MAX_SHARED_REGIONS];

// Mutexes and locks
pthread_condattr_t cond_attrs[MAX_SHARED_PAGES];
pthread_cond_t conds[MAX_SHARED_PAGES];
pthread_mutex_t mutexes[MAX_SHARED_PAGES];

// Functions
int dsmlib_init(char *ip, int port, uintptr_t start, size_t length) {
	struct sigaction new_sig_action;

	// Register page fault handler
	new_sig_action.sa_flags = SA_SIGINFO;
	new_sig_action.sa_sigaction = (void *)page_fault_handler;
	sigemptyset(&new_sig_action.sa_mask);


	// Initialize mutexes
	for(i = 0; i < MAX_SHARED_PAGES; i++) {
		pthread_condattr_init(&cond_attrs[i]);
		pthread_cond_init(&conds[i]);
		pthread_mutex_init(&mutexes[i]);
	}

	// Initialize shared areas
	next_shared_page = 0;
	for(i = 0; i < MAX_SHARED_AREAS; i++) {
		struct shared_area a = {
			.start = 0,
			.len = 0,
		};
		shared_areas[i] = a;
	}

	// Initialize sockets
	// Spawn threads to listen to manager

	return 0;
}
 
int dsmlib_destroy(void) {
	int i;

	// Destroy mutexes and condition variables
	for(i = 0; i < MAX_SHARED_PAGES; i++) {
		pthread_condattr_destroy(&cond_attrs[i]);
		pthread_condattr_destroy(&conds[i]);
		pthread_mutex_destroy(&mutexes[i]);
	}

	// Close all sockets

	return 0;
}














