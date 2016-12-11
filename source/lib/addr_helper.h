#ifndef _ADDR_HELPER_H
#define _ADDR_HELPER_H

#include <pthread.h>
#include <stdint.h>


#define REG_ERR 19
#define PG_WRITE 0x2
#define PG_BITS 12
#define PG_SIZE (1 << (PG_BITS))


// Given an address, return the start address of that page 
static inline uintptr_t PGADDR(uintptr_t addr) {
  return addr & ~(PG_SIZE - 1);
}

// Given a page number, return the satrt address of the page 
static inline uintptr_t PGNUM_TO_PGADDR(uintptr_t pgnum) {
  return pgnum << PG_BITS;
}

// Given page address, return the page number  
static inline uintptr_t PGADDR_TO_PGNUM(uintptr_t addr) {
  return addr >> PG_BITS;
}

#endif
