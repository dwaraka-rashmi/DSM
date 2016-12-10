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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
