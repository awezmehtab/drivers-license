#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("ERROR: incomplete number of arguments\n");
		exit(EXIT_FAILURE);
	}
	pid_t cpid = fork();
	int wstatus;
	if (cpid == -1) {
		perror("ERROR: fork error\n");
		exit(1);
	} else if (cpid == 0) {
		char *args[3];
		args[0] = strdup(argv[1]);
		args[1] = strdup(argv[2]);
		args[2] = (char *)NULL;
		execvp(args[0], args);
	} else {
		// int w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
		int w = wait(&wstatus);
		if (w == -1) {
			printf("ERROR: Could not execute, status = %d\n",
					wstatus);
			exit(1);
		} else {
			printf("Command successfully completed\n");
			exit(EXIT_SUCCESS);
		}
	}
}
