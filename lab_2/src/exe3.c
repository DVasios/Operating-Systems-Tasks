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

void create_pctree(struct tree_node *tree_node, int *pid_arr) 
{
	printf("PID: %ld, name %s, starting\n", (long)getpid(), tree_node->name);
	change_pname(tree_node->name);

	if (tree_node->nr_children == 0) {

		/* Process Suspension */
		if (raise(SIGSTOP) < 0) {
			perror("Couldn't suspend process");
			exit(1);
		}
		
		/* Process Activation */
		printf("Process %s(%ld) has been activated\n", tree_node->name, (long)getpid());
		printf("%s:Sleeping...\n", tree_node->name);
		sleep(SLEEP_PROC_TREE);
		printf("Process %s(%ld) terminated!\n\n", tree_node->name, (long)getpid());
		exit(1);
	}
	else {
		/*
		 * Process Creation
		 */

		for (int i = 0; i < tree_node->nr_children; i++) {

			pid_t pid;
			//int status;
			
			pid = fork();
			if (pid < 0) {
				perror("Couldn't create child\n");
				exit(1);
			}
			else if (pid == 0) {

				int *pid_array = (int*)malloc(tree_node->nr_children * sizeof(int));
				create_pctree(tree_node->children + i, pid_array);
				exit(1);
			}

			/* Insert pid in the array */
			pid_arr[i] = pid;

			wait_for_ready_children(1);
			printf("\n");
		}

		/*
		 * Process Suspension 
		 */		

		if (raise(SIGSTOP) < 0) {
		       	perror("Couldn't suspend process");
			exit(1);
	       	}

		/*
		 * Children process activation
		 */
		
		printf("Process %s(%ld) has been activated\n", tree_node->name, (long)getpid());

		for(int i = 0; i < tree_node->nr_children; i++) {

			if(kill(pid_arr[i], SIGCONT) < 0) {
				perror("kill");
				exit(1);
			}

			/*
		 	 * Wait for the children to terminate
		 	 */

			pid_t pd;
			int sts;

			pd = wait(&sts); 
			explain_wait_status(pd, sts); 
			printf("\n");
			
		}
		printf("Process %s(%ld) terminated\n\n", tree_node->name, (long)getpid());
		exit(1);
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
	int *pid_array = (int*)malloc(root->nr_children * sizeof(int));

	pid = fork();
	if (pid < 0) {
		perror("Couldn't create child process\n");
		exit(1);
	}
	else if (pid == 0) {
		create_pctree(root, pid_array);
		exit(1);		
	}
	
	sleep(1);

	/* Tree Digram */
	printf("------------------------------------------------\n");
	printf("Every process has been suspended\n");
	printf("In five seconds I am going to show you the process tree!\n\n");
	sleep(SLEEP_PROC_TREE);
	show_pstree(pid);

	/* Continue signal to the root process */
	printf("Now the root process will be continued\n\n");
	sleep(4);
	if(kill(pid, SIGCONT) < 0) {
		perror("kill");
		exit(1);
	}

	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
