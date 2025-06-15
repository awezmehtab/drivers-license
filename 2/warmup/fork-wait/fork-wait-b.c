#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	pid_t rc = fork(), w;
	int wstatus;
	// child printing first is rare (but not impossible) in the vm you
	// might've to run many times
	if (rc < 0) {
		printf("fork failed\n");
	} else if (rc == 0) {
		printf("I am child\n");
		fflush(stdout);
	} else {
		printf("I am parent\n");
		fflush(stdout);
		w = waitpid(rc, &wstatus, 0);
	}
}
