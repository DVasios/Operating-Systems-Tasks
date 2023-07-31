#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void doWrite(int fd, const char *buff, int len)
{
	int wrcnt = write(fd, buff, len);
	
	if(wrcnt == -1){ perror("error write 1"); exit(1);}
}	
