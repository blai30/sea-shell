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
char** parse_buffer(char* buf, int* arg_c) {
    char** tokens = malloc(sizeof(char*) * 4);
    char* token = strtok(buf, " \n\t\r\v\f");
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
        tokens[*arg_c] = token;
        *arg_c += 1;
        token = strtok(NULL, " \n\t\r\v\f");
    }

    tokens[*arg_c] = NULL;  // NULL terminate array

    return tokens;
}

void execute_pipe(char** arg_v, int arg_c) {
    for (int i = 0; ; i++) {
        if (strcmp(arg_v[i], "|") == 0) {
            // Split arg_v into two arrays
            int pipe_fd[2];
            char* left_side[i];
            char* right_side[arg_c - i - 1];

            arg_v[i] = NULL;

            memcpy(left_side, arg_v, i * sizeof(char*));
            memcpy(right_side, &arg_v[i + 1], (arg_c - i - 1) * sizeof(char*));

            left_side[i] = NULL;

            pipe(pipe_fd);
            printf("1 : %d\n", getpid());
            if (fork()) {
                dup2(pipe_fd[0], 0);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
                execvp(right_side[0], right_side);
                perror("Execvp error");
            }
            printf("2 : %d\n", getpid());
            if (fork()) {
                dup2(pipe_fd[1], 1);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
                execvp(left_side[0], left_side);
                perror("Execvp error");
            }
            printf("3 : %d\n", getpid());
            break;
        }
    }
}

// Execute command and arguments
void execute(char** arg_v, int arg_c) {
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error: fork() < 0");
        exit(EXIT_FAILURE);
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
                    close(fd_out);
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
                    close(fd_out);
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
                    close(fd_in);
                    break;
                }
            }
        }

        // Execute program or throw error if it fails
        execvp(arg_v[0], arg_v);
        perror("Execvp error");
    } else {
        // The run-in-background flag '&' will prevent waiting
        if (!run_in_bg_flag) {
            waitpid(pid, &status, 0);
        }
    }
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
            exit(EXIT_SUCCESS);
        }

        int myargc = 0;
        char** myargv = parse_buffer(buffer, &myargc);

        if (myargv[0] == NULL) {
            continue;
        } else if (strcmp(myargv[0], "cd") == 0) {
            cd(myargv);
        } else if (strcmp(myargv[0], "pwd") == 0) {
            pwd();
        } else if (do_pipe) {
            execute_pipe(myargv, myargc);
        } else {
            // execvp with fork to not exit program
            execute(myargv, myargc);
        }
    }

    return 0;
}
