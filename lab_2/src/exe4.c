#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_TREE 5

void create_pctree(struct tree_node *tree_node, int fd);

/*
 * Creation of pipes
 */

int create_child(struct tree_node *child) {

	/* 
	* Create the first process of the tree
	*/
       pid_t p;
       int pfd[2];
       int status;

       printf("Creating pipe for the process (%ld)\n", (long)getpid());
       if (pipe(pfd) < 0) {
	       perror("pipe");
	       exit(1);
       }

       printf("Creating the process children\n");
       p = fork();
       if (p < 0){
	       perror("Couldn't create process");
	       exit(1);
       }
       if (p == 0) {
	       create_pctree(child, pfd[1]);
	       exit(1);
       }
       
       /* Wait for the child process */
       p = wait(&status);
       explain_wait_status(p, status);
       printf("\n");

       return pfd[0];

}


/*
 * Create process tree function 
 */

void create_pctree(struct tree_node *tree_node, int fd) 
{
	printf("Process %s(%ld) started!\n", tree_node->name, (long)getpid());
	change_pname(tree_node->name);

	if (tree_node->nr_children == 0) {
		
		printf("Getting the value from the process %s(%ld)\n", tree_node->name, (long)getpid());
		
		int val;
		sscanf(tree_node->name, "%d", &val);
		printf("The value is: %d\n", val);

		if (write(fd, &val, sizeof(val)) != sizeof(val)) {
			perror("Couldn't write to the pipe:(\n");
			exit(1);
		}

		exit(1);
	}

	else {
		/*
		 * Creating pipes for the parent-children and get the file descriptor
		 */

		int pfd1 = create_child(tree_node->children);
		int pfd2 = create_child(tree_node->children + 1);

		/* Read from child one */
		int val1, val2;
		if (read(pfd1, &val1, sizeof(val1)) != sizeof(val1)) {
			perror("Couldn't write from the pipe");
			exit(1);
		}

		/* Read from child two */
		if (read(pfd2, &val2, sizeof(val2)) != sizeof(val2)) {
			perror("Couldn't reaf from the pipe");
			exit(1);
		}
		
		/* SUM */
		if (*tree_node->name == '+') {

			int result = val1 + val2;
			if (write(fd, &result, sizeof(result)) != sizeof(result)) {
				perror("Couldn't write to the pipe");
				exit(1);
			}
		}
		/* MULTIPLY */
		else {
			int result = val1 * val2;
			if (write(fd, &result, sizeof(result)) != sizeof(result)) {
				perror("Couldn't write to the pipe");
				exit(1);
			}
		}

		printf("Process %s terminated\n\n", tree_node->name);
	}
} 

int main(int argc, char *argv[])
{
	struct tree_node *root;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);
	print_tree(root);

	/*
	 * Create process tree and pipe
	 */
	
	pid_t pid;
	int status;
	int pfd[2];
	int final_result;

	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	}

	pid = fork();
	if (pid < 0) {
		perror("Couldn't create child process\n");
		exit(1);
	}
	else if (pid == 0) {
		create_pctree(root, pfd[1]);
		exit(1);		
	}

	pid = wait(&status);
	explain_wait_status(pid, status);

	if (read(pfd[0], &final_result, sizeof(final_result)) != sizeof(final_result)) {
		perror("Couldn't read from the pipe");
		exit(1);
	}

	printf("\n\nThe final result is: %d\n", final_result);
	return 0;
}
