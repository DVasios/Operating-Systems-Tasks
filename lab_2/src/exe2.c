#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_TREE 5

/*
 * Create process tree function 
 */

void create_pctree(struct tree_node *tree_node) 
{
	printf("Process %s(%ld) started!\n", tree_node->name, (long)getpid());
	change_pname(tree_node->name);

	if (tree_node->nr_children == 0) {
		
		printf("%s:Sleeping...\n", tree_node->name);
		sleep(SLEEP_PROC_TREE);
		printf("Process %s(%ld) terminated!\n", tree_node->name, (long)getpid());
		exit(1);
	}
	else {
		for (int i = 0; i < tree_node->nr_children; i++) {

			pid_t pid;
			int status;
			
			pid = fork();
			if (pid < 0) {
				perror("Couldn't create child\n");
				exit(1);
			}
			else if (pid == 0) {
				create_pctree(tree_node->children + i);
				exit(1);
			}

			pid = wait(&status);
			explain_wait_status(pid, status);
			printf("\n");

		}
		printf("Process %s(%ld) terminated\n\n", tree_node->name, (long)getpid());
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
	 * Create process tree
	 */
	
	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0) {
		perror("Couldn't create child process\n");


		pid_t p;
		exit(1);
	}
	else if (pid == 0) {
		create_pctree(root);
		exit(1);		
	}

	pid = wait(&status);
	explain_wait_status(pid, status);
	return 0;
}
