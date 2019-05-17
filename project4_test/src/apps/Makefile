CC = cc

CLEANFILES = $(PROGS) *.o

CFLAGS = -O2 -pipe
CFLAGS +=  -Wall -Wunused-function
CFLAGS += -Wextra

LDFLAGS = -lm

all: cmp.o
	$(CC) $(CFLAGS) -o cmp cmp.o $(LDFLAGS)


clean:
	-@rm -rf $(CLEANFILES)
