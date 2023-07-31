fconc: write_file.o fconc.o
	gcc fconc.o write_file.o doWrite.o -o fconc

fconc.o: fconc.c
	gcc -Wall -c fconc.c

write_file.o: write_file.c doWrite.c 
	gcc -Wall -c write_file.c doWrite.c





