#include "encode.h"
#include "dsmlib.h"
#include "addr_helper.h"
#include "rpc.h"

// Old signal handler
static struct sigaction old_sig_action;

// Global lock
pthread_mutex_t lock;

// Shared areas
int next_shared_page = 0;
struct shared_area shared_areas[MAX_SHARED_AREAS];

// Mutexes and locks
pthread_condattr_t cond_attrs[MAX_SHARED_PAGES];
pthread_cond_t conds[MAX_SHARED_PAGES];
pthread_mutex_t mutexes[MAX_SHARED_PAGES];

// Thread that listens to the manager
static pthread_t listener_thread;

// Functions

int in_shared_addr(void *addr) {
	int i;
	int uaddr = (uintptr_t)addr;
  	for (i = 0; i < next_shared_page; i++) {
    	if ((uaddr >= shared_areas[i].start) &&
			(uaddr < PGADDR(shared_areas[i].start + shared_areas[i].length + PG_SIZE))) {
      	return 1;
    	}
  	}
  	return 0;
}

int writehandler(void *pg) {
	int page_number = PGADDR_TO_PGNUM((uintptr_t) pg);

	// Need to lock to use our condition variable.
	pthread_mutex_lock(&mutexes[page_number % MAX_SHARED_PAGES]); 

	if(request_page(page_number, "WRITE") != 0) {
		pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]);
		return -1;
	}

	// Wait for page message from server.
	pthread_cond_wait(&conds[page_number % MAX_SHARED_PAGES], 
		&mutexes[page_number % MAX_SHARED_PAGES]); 

	// Unlock, allow another handler to run.
	pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]); 

	return 0;
}

int readhandler(void *pg) {
	int page_number = PGADDR_TO_PGNUM((uintptr_t) pg);

	// Need to lock to use our condition variable.
	pthread_mutex_lock(&mutexes[page_number % MAX_SHARED_PAGES]); 

	if(request_page(page_number, "READ") != 0) {
		pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]);
		return -1;
	}

	// Wait for page message from server.
	pthread_cond_wait(&conds[page_number % MAX_SHARED_PAGES], 
		&mutexes[page_number % MAX_SHARED_PAGES]); 

	// Unlock, allow another handler to run.
	pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]); 

	return 0;
}


void page_fault_handler(int signum, siginfo_t *siginfo, ucontext_t *cont) {
	void *page_address;

	if(signum != SIGSEGV || !in_shared_addr(siginfo->si_addr)) {
		(old_sig_action.sa_handler)(signum);
	}

	page_address = (void *)PGADDR((uintptr_t) siginfo->si_addr);
	// http://stackoverflow.com/questions/17671869/how-to-identify-read-or-write-operations-of-page-fault-when-using-sigaction-hand
	if(cont->uc_mcontext.gregs[REG_ERR] & PG_WRITE) {
		// handle write fault
		if (writehandler(page_address) < 0) {
     		fprintf(stderr, "writehandler failed\n");
      		exit(1);
    	}
	} else {
		// handle read fault
		if(readhandler(page_address) < 0) {
			fprintf(stderr, "readhandler failed\n");
			exit(1);
		}
	}
}


int dsmlib_init(char *ip, int port, uintptr_t start, size_t length) {
	int i;
	struct sigaction new_sig_action;

	// Register page fault handler
	new_sig_action.sa_flags = SA_SIGINFO;
	new_sig_action.sa_sigaction = (void *)page_fault_handler;
	sigemptyset(&new_sig_action.sa_mask);


	// Initialize mutexes
	for(i = 0; i < MAX_SHARED_PAGES; i++) {
		pthread_condattr_init(&cond_attrs[i]);
    	pthread_cond_init(&conds[i], &cond_attrs[i]);
    	pthread_mutex_init(&mutexes[i], NULL);
	}

	// Initialize shared areas
	next_shared_page = 0;
	for(i = 0; i < MAX_SHARED_AREAS; i++) {
		struct shared_area a = {
			.start = 0,
			.length = 0,
		};
		shared_areas[i] = a;
	}

	// Initialize sockets
	initSocket(ip, port);

	// Spawn threads to listen to manager
	if ((pthread_create(&listener_thread, NULL, listener, NULL) != 0)) {
    	fprintf(stderr, "failed to spawn listener thread\n");
    	return -1;
  	}

	return 0;
}
 
int dsmlib_destroy(void) {
	int i;

	// Destroy mutexes and condition variables
	for(i = 0; i < MAX_SHARED_PAGES; i++) {
		pthread_condattr_destroy(&cond_attrs[i]);
    	pthread_cond_destroy(&conds[i]);
    	pthread_mutex_destroy(&mutexes[i]);
	}

	// Close all sockets
	destroySocket();

	return 0;
}














