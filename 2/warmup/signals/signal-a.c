#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	pid_t cpid;
	cpid = fork();
	if (cpid == -1) {
		perror("fork error\n");
		exit(EXIT_FAILURE);
	} else if (cpid == 0) {
		printf("I am the child and I'm gonna sleep, pid = %d\n",
				getpid());
		for (int i = 0; i < 300; i++) {
			printf(".");
			fflush(stdout);
			sleep(1);
		}
		printf("am i still alive\n");
	} else {
		printf("I am parent, I'll sleep less, pid = %d\n", getpid());
		for (int i = 0; i < 20; i++) {
			printf("p");
			fflush(stdout);
			sleep(1);
		}
		int s = kill(cpid, SIGTERM);
		if (s == -1) {
			printf("Kill failed\n");
			exit(EXIT_FAILURE);
		}
		printf("Killed the child hehe\n");
		for (int i = 0; i < 20; i++) {
			printf("p");
			fflush(stdout);
			sleep(1);
		}
		s = waitpid(cpid, NULL, 0);
		if (s == -1) {
			printf("Couldn't reap the child\n");
		}
		printf("Reaped the child\n");
		exit(EXIT_SUCCESS);
	}
}
