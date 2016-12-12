#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "dsmlib.h"
#include "addr_helper.h"

#define SIZE 2048
#define SEED 69

typedef int matrix_t [SIZE][SIZE];

int randint() {
  return rand() % 10;
}

void print_matrix(matrix_t m) {
  int i, j;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      printf(" %d ", m[i][j]);
    }
    printf("\n");
  }
}

matrix_t A, B;

int id;

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("Usage: main MANAGER_IP MANAGER_PORT id[1|2|...|n] nodes[n]\n");
    return 1;
  }

  srand(SEED);

  // manager ip
  char *ip = argv[1];
  // manager port
  int port = atoi(argv[2]);
  // Id of the node 
  id = atoi(argv[3]);
  // number of instances
  int n = atoi(argv[4]);

  // get the library running
  dsmlib_init(ip, port, 0x12340000, 4096 * 10000);


  // to get elapsed time 
  // inspired by http://stackoverflow.com/questions/12722904/how-to-use-struct-timeval-to-get-the-execution-time
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double start_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

  matrix_t *C = (matrix_t *) 0x12340000;

  int i, j, k;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      A[i][j] = randint();
      B[i][j] = randint();
    }
  }

  double factor = SIZE / n;
  int counter = 0;

    printf("Initial value %d and size %d and factor %f and n %d and id %d", counter , SIZE , factor , n , id );

  for (i = 0; i < SIZE; i++) {
    if (i >= id * factor || i < (id - 1) * factor) {
	// printf ("Continuing");
      continue;
    }

    counter++;
    printf("Initial value %d", counter);

    for (j = 0; j < SIZE; j++) {
      for (k = 0; k < SIZE; k++) {
        int temp = A[i][k] * B[k][j];
        (*C)[i][j] += temp;
      }
    }
  }

  printf("Processor id %d did %d rows\n", id, counter);

  gettimeofday(&tv, NULL);
  double end_time = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

  printf("TOTAL TIME (ms): %lf\n", (end_time - start_time));
  printf("done\n");

  printf("sleeping for 10 seconds...\n");
  sleep(10);

  /*
  printf("Matrix A\n--------\n");
  print_matrix(A);
  printf("Matrix B\n--------\n");
  print_matrix(B);
  printf("Matrix C\n--------\n");
  print_matrix(*C);
  */

  dsmlib_destroy();
  return 0;
}
