/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

sem_t *s;

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 48;
int x_chars = 90;


/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

// Struct for the threads
struct thread_info {
	int line, threads;
};

/* 
 * Thread that creates the mandel lines according to the value n
 */
void* mandel_lines_n(void *arg) {
	

	// Pass the arguments in local variables
	struct thread_info *ar = arg;
		
	// Insert the values in the struct	
	int line = ar->line;
	int color_val[x_chars];
	
	// Critical Section	
	int i, curr_line;
	for (i = 0, curr_line = line; curr_line < y_chars; i++,
				curr_line = line + i * ar->threads) {

		compute_mandel_line(curr_line, color_val);

		// This is where synchronization happens
		sem_wait(&s[line]);
	
		output_mandel_line(1, color_val);

		sem_post((line == ar->threads-1) ? &s[0] : &s[line+1]);

	}
	
	return NULL;
}


int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: ./mandel arg,\n where arg is the number of threads\n");
		return -1;
	}

	// Convert the argument to a integer
	int NTHREADS = atoi(argv[1]);
	
	// Cyclical array of semaphores
	s = calloc(NTHREADS, sizeof(sem_t));

	// Initialization of the semaphores
	int i;
	for (i = 0; i < NTHREADS; i++) {
		sem_init(&s[i], 0, i == 0 ? 1 : 0);
	}

	// Array of structs that have the necessary threads info
	struct thread_info *inf;
	inf = calloc(NTHREADS, sizeof(struct thread_info));
	
	// Array of thread names
	pthread_t thrds[NTHREADS];

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	// Create threads that computes 
	for (i = 0; i < NTHREADS; i++) {
		inf[i].line = i;
		inf[i].threads = NTHREADS;
			
		if (pthread_create(&thrds[i], NULL, mandel_lines_n, &inf[i]) != 0) {
			perror("Couldn't create thread");
			exit(1);
		}
	}

	/*
	 * Wait for all threads to terminate
	 */
	for (i = 0; i < NTHREADS; i++) {
		if(pthread_join(thrds[i], NULL) != 0) {
			perror("Couldn't join the thread\n");
			exit(1);
		}	
	}

	for (i = 0; i < NTHREADS; i++) {
		sem_destroy(&s[i]);
	}
	

	reset_xterm_color(1);
	return 0;
}
