#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	char *args[argc + 1];
	args[0] = "ls";
	args[argc] = (char *)NULL;
	for (int i = 1; i < argc; i++) {
		args[i] = strdup(argv[i]);
	}
	execvp(args[0], args);
}
