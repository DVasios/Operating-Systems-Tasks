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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

/*
 * Double array 
 */
int **rows;

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
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

void compute_and_output_mandel_line(int fd, int line, int PROCS)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */

	int i, curr_line;
	for(i = 0, curr_line = line; curr_line < y_chars; i++,
					  		  curr_line = line + i * PROCS) {
		
		compute_mandel_line(curr_line, rows[curr_line]);

	}
}

/*
 * Create a shared memory area, usable by all descendants of the calling
 * process.
 */
void *create_shared_memory_area(unsigned int numbytes)
{
	int pages;
	void *addr;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	/* Create a shared, anonymous mapping for this number of pages */

	addr = mmap(NULL, pages * sysconf(_SC_PAGE_SIZE),
			 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 

	return addr;
}

void destroy_shared_memory_area(void *addr, unsigned int numbytes) {
	int pages;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	if (munmap(addr, pages * sysconf(_SC_PAGE_SIZE)) == -1) {
		perror("destroy_shared_memory_area: munmap failed");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: ./mandel arg, \n where arg is the number of processes\n");
		return 0;
	}

	int line;

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	// Convert the argument to an integer
	int NPROCS = atoi(argv[1]) ;
	
	// Calloc for the double array
	int i;
	rows = calloc(y_chars, sizeof *rows);
	for (i = 0; i < y_chars; i++) {
		rows[i] = calloc(x_chars, sizeof(int*));
	}

	// Mapping of the arrays
	for (i = 0; i < y_chars; i++) {	
		rows[i] = create_shared_memory_area(x_chars);
	}
	
	// PIDs and Statuses of the processes
	pid_t *p;
	int *s;
	p = calloc(NPROCS, sizeof(pid_t));
	s = calloc(NPROCS, sizeof(int));
	
	// Create the processes 
	for (line = 0; line < NPROCS; line++) {
		p[line] = fork();
		if (p[line] < 0) { 
			perror("Couldn't create the process\n");
			exit(1);
		}
		if (p[line] == 0) {

			compute_and_output_mandel_line(1, line, NPROCS);
			exit(1);
		}
	}

	// Wait for the processes to finish
	for (i = NPROCS; i > 0; i--) {
		waitpid(p[i], &s[i], 0);	
	}
		
	// Print the mandebrot
	for (i = 0; i < y_chars; i++) {	
		output_mandel_line(1, rows[i]);
	}
	

	// Destroy memory area
	for (i = 0; i < y_chars; i++) {
		destroy_shared_memory_area(rows[i], NPROCS);
	}
		
	reset_xterm_color(1);
	return 0;
}
