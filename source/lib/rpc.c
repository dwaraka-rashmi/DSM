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

#include "addr_helper.h" //page adrress handler functions
#include "msgPkg.h" //data encode and decode functions


// int initSocket(char *ip, int port);
// int destroySocket(void);
// int listener(void *ptr);
// int messageHandler(char *payload);
// int invalidate(char *payload);
// void confirmInvalidateEncoded(int pgnum, char *encodedPage);
// void confirmInvalidate(int pgnum);
// int sendMessage(char *message);
// int handlePageRequest(char *msg);
// int requestpage(int pgnum, char *type);

//socket state
int socketfd;
pthread_mutex_t socketLock;

/* refer man getaddrinfo
//    struct addrinfo {
//       int              ai_flags;
//       int              ai_family;
//       int              ai_socktype;
//       int              ai_protocol;
//       socklen_t        ai_addrlen;
//       struct sockaddr *ai_addr;
//       char            *ai_canonname;
//       struct addrinfo *ai_next;
//   }; */
struct addrinfo hints;
struct addrinfo *resolvedAddr;

extern pthread_condattr_t waitca[MAX_SHARED_PAGES];
extern pthread_cond_t waitc[MAX_SHARED_PAGES];
extern pthread_mutex_t waitm[MAX_SHARED_PAGES];


/*initialize the socket connection
// input: IP and Port address of the centralized manager  */
int initSocket(char *ip, int port) {

	char managerPort[5];
    snprintf(managerPort, 5, "%d", port);

	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;  /* change it to AF_UNSPEC to allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;  /* Any protocol */


    if(getaddrinfo(ip, managerPort, &hints, &resolvedAddr) < 0) {
    	fprintf(stderr, "IP - Port cannot be resolved.. \n");
    	return -2;
  	}

  	socketfd = socket(resolvedAddr->ai_family, resolvedAddr->ai_socktype, resolvedAddr->ai_protocol);
	if(socketfd < 0) {
    	fprintf(stderr, "Invalid Socket fd..\n");
    	freeaddrinfo(resolvedAddr); /* free the address */
    	return -2;
 	}

    if(connect(socketfd, resolvedAddr->ai_addr,resolvedAddr->ai_addrlen) < 0) {
    	fprintf(stderr, "Unable to connect to the given IP address\n");
    	freeaddrinfo(resolvedAddr);
    	return -2;
  	}

  	freeaddrinfo(resolvedAddr);

  	if(pthread_mutex_init(&socketLock, NULL) != 0) {
    	return -3;
  	}

  	return 0; 	

}

/* Destroy the Socket connection with the centralized manager */
int destroySocket(void) {
  if (pthread_mutex_destroy(&socketLock) != 0) {
    return -3;
  }
  close(socketfd);
  return 0;
}

/* Listen for messages from the centralized manager and handle. */
int listener(void *ptr){

	int res;

	while(1) {

		//Get the Message (Payload) length
		char payloadStr[20] = {0};
		res = recv(socketfd, payloadStr, 10, MSG_PEEK | MSG_WAITALL);
		if(res != 10){
			err(1, "unable to access the payload size");
		}
		int payloadLength = atoi(payloadStr);

		//Compute the Header length
		int headerLength;
		char *tmp = payloadStr;
		for(headerLength = 1; *tmp != ' '; tmp++)
			headerLength++;

		//Read the entire message from the socket
		char message[10000] = {0};
		res = recv(socketfd,message,headerLength + payloadLength, MSG_WAITALL);
		if(res ! = headerLength + payloadLength){
			err(1,"Unable to read from the socket");
		}

		//const char s[] = " ";
		char *payload = strstr(message, " ") + 1;
		messageHandler(payload);

	
	}

}

/* Registered the Message Handler here */
int messageHandler(char *payload){

	if(strstr(payload,"INVALIDATE") != NULL)
		invalidate(payload);
	else if(strstr(payload, "REQUESTPAGE") != NULL)
		handlePageRequest(payload); 
	else printf("Undefined Message\n");

	return 0;
}

// Handle invalidate messages.
int invalidate(char *payload) {

  int err;
  char *extractPg = strstr(payload, " ") + 1;
  int pgnum = atoi(extractPg);
  void *pgAddr = (void *)PGNUM_TO_PGADDR((uintptr_t)pgnum);


  /* Plain Invalidate Page request */
  if (strstr(payload, "PAGEDATA") == NULL) {
    if ((err = mprotect(pgAddr, 1, PROT_NONE)) != 0) {
      fprintf(stderr, "Page adrress %p invalidation failed with error %d\n", pgAddr, err);
      return -1;
    }
    confirmInvalidate(pgnum);
    return 0;
  }

  /* Page Data is requested */
  if ((err = mprotect(pgAddr, 1, PROT_READ)) != 0) {
      fprintf(stderr, "Page adrress %p invalidation failed with error %d\n", pgAddr, err);
      return -1;
  }
  
  char encodedPage[PG_SIZE * 2] = {0};
  base64Encode((const char *)pgAddr, PG_SIZE, encodedPage);
  if ((err = mprotect(pgAddr, 1, PROT_NONE)) != 0) {
      fprintf(stderr, "Page adrress %p invalidation failed with error %d\n", pgAddr, err);
      return -1;
  }

  confirmInvalidateEncoded(pgnum, encodedPage);
  return 0;

}

//Format Message with encoded data
void confirmInvalidateEncoded(int pgnum, char *encodedPage) {
  char message[1000] = {0};
  snprintf(message, 100 + strlen(encodedPage), "INVALIDATE CONFIRMATION %d %s", pgnum, encodedPage);
  sendMessage(message);
}

//Format Message
void confirmInvalidate(int pgnum) {
  char message[1000] = {0};
  snprintf(message, 100, "INVALIDATE CONFIRMATION %d", pgnum);
  sendMessage(message);
}

// Send the message to the centralized manager.
int sendMessage(char *message) {
  
  int res;

  pthread_mutex_lock(&socketLock);
  char msg[1000];
  sprintf(msg, "%zu %s", strlen(message), message);
  
  res = send(socketfd, msg, strlen(msg), 0);
  if (res != strlen(msg))
    err(1, "error in the message");
  pthread_mutex_unlock(&socketLock);
  
  return 0;
}


int handlePageRequest(char *msg) {

  char *extractPg = strstr(msg, "ION ") + 4;
  int pgnum = atoi(extractPg);
  void *pageAddr = (void *)PGNUM_TO_PGADDR((uintptr_t)pgnum);

  int err;
  int nspaces;

  // Acquire mutex lock for condition variable.
  pthread_mutex_lock(&waitm[pgnum % MAX_SHARED_PAGES]);

  /* If EXISTING , decode the encoded data. 
  		Data begins after the 4th ' ' character in msg. */
  if (strstr(msg, "EXISTING") == NULL) {
    char *encodedPage = msg;
    nspaces = 0;
    while (nspaces < 4) {
      if (encodedPage[0] == ' ') {
	      nspaces++;
      }
      encodedPage++;
    }
    
    char encodedData[7000];
    if (base64Encode(encodedPage, encodedData) < 0) {
      fprintf(stderr, "Failure decoding the encodedPage");
      return -1;
    }

    // memcpy -- must set to write first to fill in page!
    if ((err = mprotect(pg, 1, (PROT_READ|PROT_WRITE))) != 0) {
      fprintf(stderr, "Permission Alteration failed with error %d\n", err);
      return -1;
    }
    if (memcpy(pg, encodedData, PG_SIZE) == NULL) {
      fprintf(stderr, "memcpy failed.\n");
      return -1;
    }
  }

  //Write
  if (strstr(msg, "WRITE") == NULL) {
    if ((err = mprotect(pg, 1, PROT_READ)) != 0) {
      fprintf(stderr, "Permission Alteration failed with error %d\n", err);
      return -1;
    }
  } else {
    if ((err = mprotect(pg, 1, PROT_READ|PROT_WRITE)) != 0) {
      fprintf(stderr, "Permission Alteration failed with error %d\n", err);
      return -1;
    }
  }

  // Signal the page handler
  pthread_cond_signal(&waitc[pgnum % MAX_SHARED_PAGES]);

  // Unlock to handle page faults in the queue.
  pthread_mutex_unlock(&waitm[pgnum % MAX_SHARED_PAGES]); 
  return 0;


}

//Invoked from the client
int requestpage(int pgnum, char *type) {
  char msg[100] = {0};
  snprintf(msg, 100, "REQUESTPAGE %s %d", type, pgnum);
  return sendMessage(msg);
}