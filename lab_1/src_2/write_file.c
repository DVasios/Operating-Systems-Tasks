#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "doWrite.h"

void write_file(int fd, const char *infile)
{
	char buff[1024];
	int rcnt = -1;

	int fd_file = open(infile, O_RDONLY);

 	while (rcnt != 0)	
	{ 	

		rcnt = read(fd_file, buff, sizeof(buff) - 1);
	
		if(rcnt == -1){perror("error read 2"); exit(2);}
		
		doWrite(fd, buff, rcnt);
	}
	
	close(fd_file);
}
