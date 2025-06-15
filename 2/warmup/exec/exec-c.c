#include <stdio.h>
#include <unistd.h>

int main() {
	printf("trying exec (execlp specifically)\n");
	int ret = execlp("asdfman", "adsf13413", NULL);
	printf("here's the ret value: %d\n", ret);
	printf("this isn't printed unless execlp fails, which is when exec"
			" can't find the executable you've given in the route"
			" specified by the $PATH variable\n");
}
