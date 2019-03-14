/****************************************************************
 * Name        :  Brian Lai                                     *
 * Class       :  CSC 415                                       *
 * Date        :  February 27, 2019                             *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/* --------------------*/
#define PROMPT "myShell %s >> "
#define PROMPTSIZE sizeof(PROMPT)

int run_in_bg_flag;
int rd_output;
int rd_output_append;
int rd_input;
int do_pipe;
char* rd_file;

void clear() {
    printf("\033[H\033[J");
}

void print_dir() {
    char cur_dir[1024];
    char* home_path = getenv("HOME");
    getcwd(cur_dir, sizeof(cur_dir));

    // Replace home path with ~ when printing current working directory
    char *p;
    if ((p = strstr(cur_dir, home_path))) {
        char new_str[1024];
        strncpy(new_str, cur_dir, p - cur_dir);
        new_str[p - cur_dir] = '\0';
        sprintf(new_str + (p - cur_dir), "%s%s", "~", p + strlen(home_path));
        printf(PROMPT, new_str);
    } else {
        // Print current working directory full path
        printf(PROMPT, cur_dir);
    }
}

// Change current working directory
void cd(char** arg_v) {
    if (!arg_v[1]) {
        printf("Enter a directory for cd\n");
    } else if (strcmp(arg_v[1], "~") == 0) {
        chdir(getenv("HOME"));
    } else {
        chdir(arg_v[1]);
    }
}

// Print current working directory full path
void pwd() {
    char pwd[1024];
    printf("%s\n", getcwd(pwd, sizeof(pwd)));
}

// Parse buffer and count arguments
char** parse_buffer(char* buf) {
    char** tokens = malloc(sizeof(char*) * 4);
    char* token = strtok(buf, " \n\t\r\v\f");
    int index = 0;
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            rd_output = 1;
        } else if (strcmp(token, ">>") == 0) {
            rd_output_append = 1;
        } else if (strcmp(token, "<") == 0) {
            rd_input = 1;
        } else if (strcmp(token, "|") == 0) {
            do_pipe = 1;
        } else if (strcmp(token, "&") == 0) {
            run_in_bg_flag = 1;
            break;
        }
        // arg_c starts at 0
        tokens[index] = token;
        index++;
        token = strtok(NULL, " \n\t\r\v\f");
    }

    tokens[index] = NULL;  // NULL terminate array

    return tokens;
}

// Count the number of args in the array
int count_argc(char** arg_v) {
    int count = 0;
    for (int i = 0; arg_v[i] != NULL; i++) {
        count++;
    }
    return count;
}

// Split the array into arrays used for pipe. The offset parameter allows this function to be reused
char** divide_argv(char** arg_v, int* offset) {
    char** new_argv = malloc(sizeof(char*) * 4);

    int i;
    for (i = *offset; strcmp(arg_v[i], "|") != 0; i++) {
        if (arg_v[i] == NULL) {
            break;
        }
        new_argv[i] = arg_v[i];
    }
    new_argv[i] = NULL;

    *offset = i;
    return new_argv;
}

int execute_pipe(char** left_side, char** right_side) {
//    for (int i = 0; ; i++) {
//        if (strcmp(arg_v[i], "|") == 0) {
            // Split arg_v into two arrays
            int pipe_fd[2]; /* [0] read end [1] write end */
//            int left_argc = i;
//            int right_argc = arg_c - i - 1;
//            char* left_side[left_argc];
//            char* right_side[right_argc];
            int status;
            pid_t pid;

//            arg_v[i] = NULL;

//            memcpy(left_side, arg_v, left_argc * sizeof(char*));
//            memcpy(right_side, &arg_v[i], right_argc * sizeof(char*));
//
//            left_side[left_argc] = NULL;
//            right_side[right_argc] = NULL;

            pipe(pipe_fd);
            pid = fork();

            if (pid == 0) {
                dup2(pipe_fd[0], 0);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
                execvp(right_side[0], right_side);
                perror("Execvp error");
                return -1;
            } else if (pid < 0) {
                perror("Fork error: fork() < 0");
                return -1;
            } else {
                if (!run_in_bg_flag) {
                    waitpid(pid, &status, 0);
                }
            }

            pid = fork();

            if (pid == 0) {
                dup2(pipe_fd[1], 1);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
                execvp(left_side[0], left_side);
                perror("Execvp error");
                return -1;
            } else if (pid < 0) {
                perror("Fork error: fork() < 0");
                return -1;
            } else {
                if (!run_in_bg_flag) {
                    waitpid(pid, &status, 0);
                }
            }

//            break;
//        }
//    }

    return 0;
}

// Execute command and arguments
int execute(char** arg_v, int arg_c) {
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error: fork() < 0");
        return -1;
    } else if (pid == 0) {
        // Output redirection overwrite
        if (rd_output) {
            for (int i = 0; ; i++) {
                if (strcmp(arg_v[i], ">") == 0) {
                    // Filename is after >
                    rd_file = arg_v[i + 1];
                    arg_v[i] = NULL;
                    int fd_out = creat(rd_file, 0644);
                    dup2(fd_out, 1);
//                    close(fd_out);
                    break;
                }
            }
        }

        // Output redirection append
        if (rd_output_append) {
            for (int i = 0; ; i++) {
                if (strcmp(arg_v[i], ">>") == 0) {
                    // Filename is after >>
                    rd_file = arg_v[i + 1];
                    arg_v[i] = NULL;
                    int fd_out = open(rd_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(fd_out, 1);
//                    close(fd_out);
                    break;
                }
            }
        }

        // Input redirection
        if (rd_input) {
            for (int i = 0; ; i++) {
                if (strcmp(arg_v[i], "<") == 0) {
                    // Filename is after <
                    rd_file = arg_v[i + 1];
                    arg_v[i] = NULL;
                    int fd_in = open(rd_file, O_RDONLY);
                    dup2(fd_in, 0);
//                    close(fd_in);
                    break;
                }
            }
        }

        // Execute program or throw error if it fails
        execvp(arg_v[0], arg_v);
        perror("Execvp error");
        return -1;
    } else {
        // The run-in-background flag '&' will prevent waiting
        if (!run_in_bg_flag) {
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}

// the project came as int* argc but Souza confirmed it should be int argc
int main(int argc, char** argv) {

    clear();

    while (1) {
        // Reset buffer and flag
        char buffer[BUFFERSIZE];
        run_in_bg_flag = 0;
        rd_output = 0;
        rd_output_append = 0;
        rd_input = 0;
        do_pipe = 0;
        rd_file = NULL;

        print_dir();

        // Read line of input
        fgets(buffer, BUFFERSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;    // Clear \n

        // End program
        if (strcmp(buffer, "exit") == 0) {
            return 0;
        }

        char** myargv = parse_buffer(buffer);
        int myargc = count_argc(myargv);

        if (myargv[0] == NULL) {
            continue;
        } else if (strcmp(myargv[0], "cd") == 0) {
            cd(myargv);
        } else if (strcmp(myargv[0], "pwd") == 0) {
            pwd();
        } else if (do_pipe) {
            int offset = 0;
            char** left_argv = divide_argv(myargv, &offset);
            printf("%d", offset);
            char** right_argv = divide_argv(myargv, &offset);
            printf("%d", offset);
            int left_argc = count_argc(left_argv);
            int right_argc = count_argc(right_argv);

            if (execute_pipe(left_argv, right_argv) != 0) {
                printf("execute_pipe failed");
            }
        } else {
            // execvp with fork to not exit program
            if (execute(myargv, myargc) != 0) {
                printf("execute failed");
            }
        }

        free(myargv);
    }

    return 0;
}
