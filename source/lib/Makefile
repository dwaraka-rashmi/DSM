CC = gcc
INCLUDES = -I../include
CFLAGS = -Wall $(INCLUDES)
LFLAGS =
LIBS = -lb64 -lpthread

TESTS = main matrixmultiply2
SRCS = encode.c dsmlib.c rpc.c
OBJS = $(SRCS:.c=.o)

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

.PHONY: depend clean

all: $(TESTS)
	@echo Build complete.

main: main.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LFLAGS) $(LIBS)

matrixmultiply2: matrixmultiply2.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LFLAGS) $(LIBS)

matrix: matrix.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LFLAGS) $(LIBS) 

.c: .o
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *~ $(TESTS)

depend:
	$(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend depends on it

