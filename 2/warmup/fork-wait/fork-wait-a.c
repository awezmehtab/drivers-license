#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 4

int main() {
    // this makes 2^n - 1 children instead
    pid_t curr = getpid();
    printf("started with pid = %d\n", curr);
    for (int i = 0; i < N; i++) {
        fflush(stdout);
        int cpid = fork();
        if (cpid == 0) {
            printf("new process spawned at i = %d, pid = %d\n", i, getpid());
            // } else {
            //     int w = waitpid(cpid, NULL, 0);
            //     if (w == -1) {
            //         printf("wait error at i = %d\n", i);
            //         exit(1);
            //     }
        }
    }

    /* Creates exactly N processes
    pid_t curr = getpid();
    printf("init process i = %d, pid = %d\n", 0, getpid());

    for (int i = 1; i < N; i++) {
        pid_t cpid = fork();
        if (cpid == -1) {
            printf("fork error at i = %d\n", i);
            exit(1);
        } else if (cpid == 0) {
            printf("new child at i = %d, pid = %d\n", i, getpid());
        } else {
            int w = waitpid(cpid, NULL, 0);
            if (w == -1) {
                printf("wait error at i = %d\n", i);
                exit(1);
            }
            break;
        }
    }
    */
}
