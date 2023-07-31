#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/prctl.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  20
#define SLEEP_TREE_SEC  3

/* Code of process B*/
void fork_B(void)
{	
	printf("--->Process B is starting!\n");
	change_pname("B");

	/*
	 * Create process D
	 */

	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0) { 
		perror("Couldn't create process D");
		exit(1);
	}
	else if (pid == 0) {
	        change_pname("D");
		printf("--->Process D is starting!\n");
       		printf("--->D: Sleeping...\n");
       		sleep(SLEEP_PROC_SEC);
       		exit(13);

	}
	
	pid = wait(&status);
	explain_wait_status(pid, status);
	exit(19);
}



void fork_procs(void)
{	
	printf("--->Process A is starting!\n");
	change_pname("A");

	/* 
	 * Create process B
	 */

	pid_t pidB;
	int statusB;

	pidB = fork();
	if (pidB < 0) {
		perror("Couldn't creat process B");
		exit(1);
	}
	else if (pidB == 0) {
		/* Process B */
		fork_B();
		exit(1);
	}

	/* 
	 * Create process C
	 */

	pid_t pidC;
	int statusC;

	pidC = fork();
	if (pidC < 0) {
		perror("Couldn't creat process C");
		exit(1);
	}
	else if (pidC == 0) {
		/* Proccess C */
		printf("--->Process C is starting!\n");
        	change_pname("C");
        	printf("--->C:Sleeping...\n\n");
        	sleep(SLEEP_PROC_SEC);
		exit(17);
	}

	pidB = wait(&statusB);
	explain_wait_status(pidB, statusB);

	pidC = wait(&statusC);
	explain_wait_status(pidC, statusC);

	exit(16);
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */
int main(void)
{
	pid_t pid;
	int status;

	/* Fork root of process tree */
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Child */
		fork_procs();
		exit(1);
	}

	/* for ask2-{fork, tree} */
	sleep(SLEEP_TREE_SEC);

	/* Print the process tree root at pid */
	show_pstree(getpid());
	show_pstree(pid);

	/* Wait for the root of the process tree to terminate */
	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
