#include <stdio.h>
#include <stdlib.h>

#include "dsmlib.h"
#include "addr_helper.h"

int main(int argc, char *argv[]) {

  // Starting library. Shared memory is the 10 pages beginning at 0x12340000.
  char *ip = argv[1]; // Manager ip
  int port = atoi(argv[2]); // Manager port
  initlibdsmu(ip, port, 0x12340000, 4096 * 10); // 10 pages shared

  // invalidate test.
  while (1) {
    int pgnum;
    printf("> ");
    scanf("%d", &pgnum);

    uintptr_t addr = PGNUM_TO_PGADDR((uintptr_t)pgnum);
    void *p = (void *)addr;
    ((char *)p)[1] = 'a';
    printf("p[1] = %c\n", ((char *)p)[1]);
  }

  dsmlib_destroy();
  return 0;
}
