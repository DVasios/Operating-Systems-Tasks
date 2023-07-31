.PHONY: all clean

all: exe1 exe2 exe3 exe4 pipe-example

CC = gcc
CFLAGS = -g -Wall -O2 -no-pie
SHELL= /bin/bash

exe1: exe1.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

exe2: exe2.o tree.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

exe3: exe3.o tree.o proc-common.o
	$(CC) $(CFLSGS) $^ -o $@

exe4: exe4.o tree.o proc-common.o
	$(CC) $(CFLSGS) $^ -o $@

pipe-example: pipe-example.o proc-common.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o tree-example fork-example pstree-this ask2-{fork,tree,signals,pipes}

