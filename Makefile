CC=gcc
CFLAGS=-O2 -Wall -Wextra -pthread
SRC_COMMON=src/common.c
HDRS=src/common.h src/barrier_api.h

all: busywait condvar

busywait: src/main_busywait.c src/barrier_busywait.c $(SRC_COMMON) $(HDRS)
	$(CC) $(CFLAGS) src/main_busywait.c src/barrier_busywait.c $(SRC_COMMON) -o busywait

condvar: src/main_condvar.c src/barrier_condvar.c $(SRC_COMMON) $(HDRS)
	$(CC) $(CFLAGS) src/main_condvar.c src/barrier_condvar.c $(SRC_COMMON) -o condvar

clean:
	rm -f busywait condvar
