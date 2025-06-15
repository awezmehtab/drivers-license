#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void int_handler(int sig, siginfo_t *si, void *unused) {
	sig = 0, si = NULL, unused = NULL;
	printf("I will never stop running man hehehe\n");
	fflush(stdout);
}

int main() {
	struct sigaction act;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = int_handler;
	if (sigaction(SIGINT, &act, NULL) == -1) {
		printf("sigaction error\n");
		exit(1);
	};

	while (1) {
		printf("haha try to stop me\n");
		sleep(1);
	}
}
