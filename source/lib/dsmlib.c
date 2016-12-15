#include "encode.h"
#include "dsmlib.h"
#include "addr_helper.h"
#include "rpc.h"

// Old signal handler
static struct sigaction old_sig_action;

// Global lock
// pthread_mutex_t lock;

// array of locks and condition variables
pthread_condattr_t cond_attrs[MAX_SHARED_PAGES];
pthread_cond_t conds[MAX_SHARED_PAGES];
pthread_mutex_t mutexes[MAX_SHARED_PAGES];

// keep track of shared pages out of the max shared pages
int next_shared_page;
struct shared_area shared_areas[MAX_SHARED_AREAS];

// Thread that listens to the manager
static pthread_t listener_thread;

// Functions

// to check if the given address exist in the currently shared segments
// return 0 if yes 
int is_shared_addr(void *addr) {
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

// handles a write page fault
int read_write_handler(int page_number,char* operation) {

	// Need to lock to use our condition variable.
	pthread_mutex_lock(&mutexes[page_number % MAX_SHARED_PAGES]); 

	int ret = request_page(page_number, operation);
	// check if the page request was successful
	if(ret == 0) {
		// Wait for page message from server.
		pthread_cond_wait(&conds[page_number % MAX_SHARED_PAGES], 
							&mutexes[page_number % MAX_SHARED_PAGES]); 

		// Unlock, allow another handler to run.
		pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]);
		return 0;

	}
	else{
		// the page request failed
		pthread_mutex_unlock(&mutexes[page_number % MAX_SHARED_PAGES]);
		return -1;
	}
}

void page_fault_handler(int signum, siginfo_t *siginfo, ucontext_t *cont) {
	void *page_address;

	//fprintf(stdout, "Inside custom page fault handler");

	// if the signal is not SIGSEGV or 
	// the address in not in between the shared addresses
	// let the original handler handle the request
	if(signum != SIGSEGV || !is_shared_addr(siginfo->si_addr)) {
		//fprintf(stdout, "address apna nhi h , jaane do");
		(old_sig_action.sa_handler)(signum);
	}

	page_address = (void *)PGADDR((uintptr_t) siginfo->si_addr);

	int page_number = PGADDR_TO_PGNUM((uintptr_t)page_address);

	// http://stackoverflow.com/questions/17671869/how-to-identify-read-or-write-operations-of-page-fault-when-using-sigaction-hand
	if(cont->uc_mcontext.gregs[REG_ERR] & PG_WRITE) {
		// handle write fault
		if (read_write_handler(page_number,"WRITE") < 0) {
     		fprintf(stderr, "writehandler failed\n");
      		exit(1);
    	}
	} else {
		// handle read fault
		if(read_write_handler(page_number,"READ") < 0) {
			fprintf(stderr, "readhandler failed\n");
			exit(1);
		}
	}
	return;
}

int dsmlib_init(char *ip, int port, uintptr_t start, size_t length) {
	int i,ret;
	struct sigaction new_sig_action;

	// Register page fault handler
	new_sig_action.sa_sigaction = (void *)page_fault_handler;
	sigemptyset(&new_sig_action.sa_mask);
	new_sig_action.sa_flags = SA_SIGINFO;
	
	if (sigaction(SIGSEGV, &new_sig_action, &old_sig_action) != 0) {
    	fprintf(stderr, "sigaction failed\n");
  	}
	
	//printf("new handler init complete");


	// Initialize mutexes
	for(i = 0; i < MAX_SHARED_PAGES; i++) {
		pthread_condattr_init(&cond_attrs[i]);
    	pthread_cond_init(&conds[i], &cond_attrs[i]);
    	pthread_mutex_init(&mutexes[i], NULL);
	}

	// Initialize shared areas
	struct shared_area a={0,0};
	next_shared_page = 0;
	for(i = 0; i < MAX_SHARED_AREAS; i++) {
		shared_areas[i] = a;
	}

	ret = add_shared_area(start, length);
	// Setup initial shared memory area.
    if (ret < 0) {
    	fprintf(stderr, "initial shared memory assignment failed\n");
    	if(ret==-1)
    		err(1, "shared region init failed because of mmap failure");
    	else if(ret==-2)
    		err(1, "Maximum shared area size reached");
    	else
    		err(1, "shared region init failed");
  	}

	// Initialize sockets
	initSocket(ip, port);

	// Spawn threads to listen to manager
	ret = pthread_create(&listener_thread, NULL, listener, NULL);
	if (ret != 0) {
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

int add_shared_area(uintptr_t start, size_t len) {
	// should not exceed maximum shared pages
	if (next_shared_page >= MAX_SHARED_PAGES) {
		fprintf(stderr, "Maximum shared areas limit reached\n");	
		return -2;
	}

	int zero_fd = open("/dev/zero", O_RDONLY, 0644);
	void *p = mmap((void *)start, len, (PROT_NONE),(MAP_ANON|MAP_PRIVATE), zero_fd, 0);
	if ((p < 0) || (p == NULL)) {
	  fprintf(stderr, "mmap failed.\n");
	  return -1;
	}

	// initial the shared area and the next shared page
    struct shared_area r = {start, len};
    shared_areas[next_shared_page] = r;
    next_shared_page++;

    return 0;
}