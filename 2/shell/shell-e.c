#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG 64

/*
 * Here are the errors that need to be fixed:
 * (1) Setting up the group id for fg and bg process
 */

/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line) {
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++) {
        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0) {
                tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        } else {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

size_t sizearr(char **s) {
    size_t res = 0;
    while (s[res]) {
        res++;
    }
    return res;
}

char *prompt_dir() {
    char *cwd = (char *)malloc(256 * sizeof(char));
    const char *home = getenv("HOME");

    if (getcwd(cwd, 256) != NULL) {
        char *res = strstr(cwd, home);
        if (res) {
            char *temp = (char *)malloc(256 * sizeof(char));
            strcpy(temp, "~");
            strcat(temp, res + strlen(home));
            strcpy(cwd, temp);
            free(temp);
        }
    }
    return cwd;
}

pid_t fgrp = -1;
int fvalid = 0;

static void handler(int sig, siginfo_t *si, void *unused) {
    if (fvalid == 1) {
        fvalid = 0;
        int status = kill(-fgrp, SIGKILL);
        printf("\n");
        if (status == -1 && errno != ESRCH) {
            printf("something wrong with kill\n");
        }
    }
}

// stores parsed commands
int parsed_cmds[MAX_NUM_TOKENS];

// returns number of tokens (commands) found
int parsecmd(char **tokens, const char *delim) {
    for (int i = 0; i < MAX_NUM_TOKENS; i++) parsed_cmds[i] = 0;
    int found = 0;
    parsed_cmds[0] = 1;
    for (int i = 0; tokens[i]; i++) {
        if (strcmp(tokens[i], delim) == 0) {
            tokens[i] = NULL;
            if (i < MAX_NUM_TOKENS - 1)
                parsed_cmds[i + 1] = 1;
            found++;
        }
    }
    return found;
}

// IMPORTANT: checks for zombie background process at each given prompt
// and kills them. bg[i] == 0 means the slot is free for a bg process
void clear_bg(pid_t *bg) {
    for (int i = 0; i < MAX_BG; i++) {
        if (bg[i] == 0)
            continue;
        int wst, bg_state = waitpid(bg[i], &wst, WNOHANG);
        if (bg_state == 0)
            continue;
        if (bg_state == -1) {
            printf("wait for child failed, child id: %d\n", bg[i]);
            exit(1);
        } else if (bg_state > 0) {
            printf("child %d exited\n", bg_state);
            bg[i] = 0;
        }
    }
}

void run_pl(int num_comm, char **tokens) {
    pid_t dpid = fork();
    fvalid = 1;
    fgrp = dpid;
    if (dpid == -1) {
        printf("couldn't create dummy parent: %s\n", strerror(errno));
    } else if (dpid == 0) {
        if (setpgid(0, 0) == -1) {
            printf("kyu bhai aur error mat dikha\n");
        }
        fgrp = getpgrp();
        pid_t pproc[MAX_NUM_TOKENS];
        int count = 0;
        for (int token_no = 0; count < num_comm; token_no++) {
            if (parsed_cmds[token_no] != 1)
                continue;
            if (token_no != 0 && fvalid == 0) {
                break;
            }
            if (fvalid == 0)
                break;
            pid_t cpid = fork();
            pproc[count++] = cpid;
            if (cpid == -1) {
                printf("fork for parallel commands failed\n");
            } else if (cpid == 0) {
                char **start = &tokens[token_no];
                execvp(start[0], start);
                printf("exec error: %s\n", strerror(errno));
                exit(1);
            }
        }

        for (int pproc_no = 0; pproc_no < num_comm; pproc_no++) {
            if (waitpid(pproc[pproc_no], NULL, 0) == -1) {
                printf("wait error in parallel execution, pid: %d\n",
                       pproc[pproc_no]);
            }
        }
        exit(0);
    } else {
        if (waitpid(dpid, NULL, 0) == -1) {
            printf("couldn't reap dummy parent\n");
            exit(1);
        }
        fvalid = 0;
    }
}

void run_sl(int num_comm, char **tokens) {
    int count = 0;
    for (int token_no = 0; count < num_comm; token_no++) {
        if (parsed_cmds[token_no] != 1)
            continue;
        count++;
        if (fvalid == 0 && token_no != 0) {
            break;
        }
        pid_t cpid = fork();
        if (token_no == 0) {
            fvalid = 1;
        }
        fgrp = cpid;
        if (fvalid == 1) {
            if (setpgid(cpid, fgrp) == -1) {
                printf("setpgid failed\n");
            };
        }
        if (cpid == -1) {
            printf("fork for multiple commands failed\n");
        } else if (cpid == 0) {
            // setting up args
            char **start = &tokens[token_no];

            // execing
            execvp(start[0], start);
            printf("one of the commands is bad\n");
            exit(1);
        } else {
            int wstatus;
            if (waitpid(cpid, &wstatus, 0) == -1) {
                printf("waitpid failed\n");
                exit(1);
            }
        }
    }
    fvalid = 0;
}

void run_redir_in(int num_comm, char **tokens) {
    int fd;
    int count = 0;
    for (int pno = 0; count < 2; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        if (count == 1) {
            char **start = &tokens[pno];
            if ((fd = open(start[0], O_RDONLY,
                           S_IRGRP | S_IWGRP | S_IWUSR | S_IRUSR | S_IROTH)) ==
                -1) {
                printf("%s: %s\n", start[0], strerror(errno));
                return;
            }
        }
        count++;
    }
    count = 0;
    pid_t cpid = fork();
    for (int pno = 0; count < 1; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        char **start = &tokens[pno];
        if (cpid == 0) {
            if (dup2(fd, STDIN_FILENO) == -1) {
                printf("dup error: %s\n", strerror(errno));
                exit(1);
            }
            close(fd);
            execvp(start[0], start);
        } else {
            close(fd);
            if (waitpid(cpid, NULL, 0) == -1) {
                printf("wait error: %s\n", strerror(errno));
                exit(1);
            }
        }
        count++;
    }
}

void run_redir_out(int num_comm, char **tokens) {
    int fd;
    int count = 0;
    for (int pno = 0; count < 2; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        if (count == 1) {
            char **start = &tokens[pno];
            fd = open(start[0], O_WRONLY | O_CREAT,
                      S_IRGRP | S_IWGRP | S_IWUSR | S_IRUSR | S_IROTH);
        }
        count++;
    }
    pid_t cpid = fork();
    count = 0;
    for (int pno = 0; count < 1; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        char **start = &tokens[pno];
        if (cpid == 0) {
            if (dup2(fd, STDOUT_FILENO) == -1) {
                printf("dup error: %s\n", strerror(errno));
                exit(1);
            }
            close(fd);
            execvp(start[0], start);
        } else {
            close(fd);
            if (waitpid(cpid, NULL, 0) == -1) {
                printf("wait error: %s\n", strerror(errno));
                exit(1);
            }
        }
        count++;
    }
}

void run_redir_app(int num_comm, char **tokens) {
    int fd;
    int count = 0;
    for (int pno = 0; count < 2; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        if (count == 1) {
            char **start = &tokens[pno];
            fd = open(start[0], O_WRONLY | O_APPEND | O_CREAT,
                      S_IRGRP | S_IWGRP | S_IWUSR | S_IRUSR | S_IROTH);
        }
        count++;
    }
    pid_t cpid = fork();
    count = 0;
    for (int pno = 0; count < 1; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        char **start = &tokens[pno];
        if (cpid == 0) {
            if (dup2(fd, STDOUT_FILENO) == -1) {
                printf("dup error: %s\n", strerror(errno));
                exit(1);
            }
            close(fd);
            execvp(start[0], start);
        } else {
            close(fd);
            if (waitpid(cpid, NULL, 0) == -1) {
                printf("wait error: %s\n", strerror(errno));
                exit(1);
            }
        }
        count++;
    }
}

void run_pipe_arr(int num_comm, char **tokens) {
    int pipefd[2], count = 0, pipefdarr[MAX_NUM_TOKENS][2];

    if (pipe(pipefd) == -1) {
        printf("pipe failure\n");
        exit(1);
    }

    for (int i = 0; i < num_comm; i++) {
        if (pipe(pipefdarr[i]) == -1) {
            printf("pipe failure at %d\n", i);
            exit(1);
        }
    }

    // p0 | p1
    // p0 -> stdout redirected to pipefd[1]
    // p1 -> stdin redirected to pipefd[0]
    // 0 1 2 ... num_comm
    for (int pno = 0; count < num_comm + 1; pno++) {
        if (parsed_cmds[pno] != 1)
            continue;
        pid_t cpid = fork();
        char **start = &tokens[pno];
        if (cpid == 0) {
            if (count > 0) {
                close(pipefdarr[count - 1][1]);
                if (dup2(pipefdarr[count - 1][0], STDIN_FILENO) == -1) {
                    printf("dup failed: %s\n", strerror(errno));
                }
                close(pipefdarr[count - 1][0]);
            }
            if (count < num_comm) {
                close(pipefdarr[count][0]);
                if (dup2(pipefdarr[count][1], STDOUT_FILENO) == -1) {
                    printf("dup failed: %s\n", strerror(errno));
                }
                close(pipefdarr[count][1]);
            }

            execvp(start[0], start);
            printf("exec failed in pipe 0\n");
            exit(1);
        } else {
            if (waitpid(cpid, NULL, 0) == -1) {
                printf("wait error\n");
                exit(1);
            }
            if (count < num_comm)
                close(pipefdarr[count][1]);
            if (count > 0)
                close(pipefdarr[count - 1][0]);
        }
        count++;
    }
}

void runcmd(char **tokens, pid_t *bg) {
    // checks if bg process can run and assigns a bg index in bg array
    int is_bg = 0, bidx;
    size_t sz = sizearr(tokens);
    is_bg = sz > 1 && strncmp(tokens[sz - 1], "&", 1) == 0;
    if (is_bg == 1) {
        int found = 0;
        for (int i = 0; i < MAX_BG; i++) {
            if (bg[i] == 0) {
                found = 1;
                bidx = i;
                break;
            }
        }
        if (found == 0) {
            printf("too many background processes\n");
            return;
        } else {
            tokens[sz - 1] = (char *)NULL;
        }
    }

    pid_t cpid = fork();
    if (is_bg && cpid > 0) {
        printf("background process started. pid = %d\n", cpid);
        bg[bidx] = cpid;
    }
    if (is_bg == 0) {
        fvalid = 1;
        fgrp = cpid;
    } else {
        fgrp = -1;
    }
    if (cpid == -1) {
        perror("could not fork\n");
        exit(1);
    } else if (cpid == 0) {
        if (is_bg == 1) {
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGINT);
            sigprocmask(SIG_BLOCK, &set, NULL);
        }
        execvp(tokens[0], tokens);
        printf("command not found\n");
        exit(1);
    } else {
        if (is_bg == 1)
            return;
        int wstatus;
        if (waitpid(cpid, &wstatus, 0) == -1) {
            printf("waitpid failed, err: %s\n", strerror(errno));
            exit(1);
        }
        fvalid = 0;
    }
}

int hfile;

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

void repl() {
    char line[MAX_INPUT_SIZE];
    char **tokens;
    int i;
    pid_t bg[MAX_BG];
    bzero(bg, sizeof(bg));

    while (1) {
        /* BEGIN: TAKING INPUT */
        char *prompt = prompt_dir();
        bzero(line, sizeof(line));
        printf("\033[33m%s \033[0m\033[35m❯\033[0m ", prompt);
        fflush(stdout);
        scanf("%[^\n]", line);
        getchar();
        free(prompt);

        // printf("Command entered: %s (remove this debug output later)\n",
        // line);
        /* END: TAKING INPUT */

        line[strlen(line)] = '\n';  // terminate with new line
        write(hfile, line, strlen(line));
        tokens = tokenize(line);

        int num_comm = parsecmd(tokens, "&&&") + 1;
        if (num_comm > 1) {
            run_pl(num_comm, tokens);
            continue;
        }

        // multiple commands at once
        num_comm = parsecmd(
            tokens, "&&");  // sorry here num_comm = number of commands - 1
        if (num_comm > 0) {
            run_sl(num_comm, tokens);
            continue;
        }

        num_comm = parsecmd(tokens, ";");
        if (num_comm > 0) {
            run_sl(num_comm, tokens);
            continue;
        }

        num_comm = parsecmd(tokens, "|");
        if (num_comm > 0) {
            run_pipe_arr(num_comm, tokens);
            continue;
        }

        num_comm = parsecmd(tokens, ">>");
        if (num_comm > 0) {
            run_redir_app(num_comm, tokens);
            continue;
        }

        num_comm = parsecmd(tokens, ">");
        if (num_comm > 0) {
            run_redir_out(num_comm, tokens);
            continue;
        }

        num_comm = parsecmd(tokens, "<");
        if (num_comm > 0) {
            run_redir_in(num_comm, tokens);
            continue;
        }

        // `exit` handler
        if (tokens[0] && strcmp(tokens[0], "exit") == 0) {
            for (int i = 0; i < MAX_BG; i++) {
                if (bg[i] != 0) {
                    if (kill(bg[i], SIGKILL) == -1) {
                        printf("kill failed\n");
                        exit(1);
                    };
                }
            }
            break;
        }

        // `cd` handler
        if (tokens[0] && strcmp(tokens[0], "cd") == 0) {
            if (chdir(tokens[1]) == -1) {
                printf("couldn't go to given directory\n");
            }
            continue;
        }

        if (tokens[0] && strcmp(tokens[0], "history") == 0) {
            char history[1000];
            pread(hfile, history, 100, max(SEEK_END - 100, 0));
            printf("%s", history);
            fflush(stdout);
            continue;
        }

        runcmd(tokens, bg);

        // Freeing the allocated memory
        for (i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}

int main(int argc, char *argv[]) {
    printf("Welcome to this shell, created by Awez (can call it ash)\n");
    hfile = open("./.ashhistory", O_RDWR | O_APPEND | O_CREAT, S_IRWXU);

    // setting SIGINT handler
    struct sigaction act;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        printf("sigaction error\n");
        exit(1);
    };

    repl();

    return 0;
}
