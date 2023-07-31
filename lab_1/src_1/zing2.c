#include <stdio.h>
#include <unistd.h>

void zing(void) 
{ 	
	printf("Hello, %s! How are you today?\n", getlogin());
}
