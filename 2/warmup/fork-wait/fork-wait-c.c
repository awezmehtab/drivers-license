#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t cpid, cst;
    int wstatus;

    cpid = fork();
    if (cpid == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (cpid == 0) {
        printf("I am child with pid %d\n", getpid());
        exit(32);
    } else {
        int w;
        if (waitpid(cpid, &w, 0) == -1) {
            printf("wait error\n");
        } else {
            printf("successfully reaped the child: %d\n", cpid);
        };
        exit(EXIT_SUCCESS);
    }
}
