CC = gcc
INCLUDES = -I../include
CFLAGS = -g -Wall $(INCLUDES)
LFLAGS =
LIBS = -lb64 -lpthread

TESTS = mmu
SRCS = encode.c dsmlib.c rpc.c
OBJS = $(SRCS:.c=.o)


.PHONY: depend clean

all: $(TESTS)
	@echo Build complete.

mmu: mmu.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LFLAGS) $(LIBS)
	
.c: .o
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *~ $(TESTS)
	clear

depend:
	$(SRCS)
	makedepend $(INCLUDES) $^

# 

