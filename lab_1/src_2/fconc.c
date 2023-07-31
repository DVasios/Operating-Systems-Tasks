#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "write_file.h"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 4) {
		printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
		return -1;
	}

	int oflags = O_CREAT | O_WRONLY | O_TRUNC;
        int mode   = S_IRUSR | S_IWUSR;
	
	char *arg;
	argc == 3 ? (arg = "fconc.out") : (arg = argv[3]);
	
	int fdnew = open(arg, oflags, mode);
	write_file(fdnew, argv[1]);
	write_file(fdnew, argv[2]);
	close(fdnew);
	
	return 0;
}
