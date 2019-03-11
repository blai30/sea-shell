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
char** parse_buffer(char* buf, int *arg_c) {
    char** tokens = malloc(sizeof(char*) * 4);
    char* token = strtok(buf, " \n\t\r\v\f");
    int count = 0;
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            rd_output = 1;
            tokens[count] = token;
        } else if (strcmp(token, "&") == 0) {
            run_in_bg_flag = 1;
            break;
        } else {
            tokens[count] = token;
        }
        count++;
        token = strtok(NULL, " \n\t\r\v\f");
    }

    tokens[count] = NULL;  // NULL terminate array
    *arg_c = count;

    return tokens;
}

// Execute command and arguments
void execute(char** arg_v, int arg_c) {
//    int in = 0;
//    int out = 1;

//    for (int i = 0; i < myargc; i++) {
//        if (strcmp(myargv[i], ">") == 0) {
//
//        }
//    }

//    if (in) {
//        int fdin = open(filein, O_RDONLY);
//        if (filein < 0) {
//            printf("Open for input failed: %s\n", filein);
//            exit(EXIT_FAILURE);
//        }
//        dup(fdin);
//        close(fdin);
//        in = 0;
//    }

//    char* fileout = myargv[2];
//    if (out) {
//        int fdout = open(fileout, O_TRUNC | O_CREAT | O_WRONLY, S_IWUSR);
//        if (fileout < 0) {
//            printf("Open for output failed: %s\n", fileout);
//            exit(EXIT_FAILURE);
//        }
//        dup(fdout);
//        close(fdout);
//        out = 0;
//    }



    // Execute normally with no redirection
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error: fork() < 0");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (rd_output) {
            for (int i = 0; ; i++) {
                if (strcmp(arg_v[i], ">") == 0) {
                    char* outfile = arg_v[i + 1];
                    printf("%s", arg_v[i + 1]);
                    arg_v[i] = NULL;
                    int fd_out = creat(outfile, 0644);
                    dup2(fd_out, 1);
                    break;
                }
            }
        }
        execvp(arg_v[0], arg_v);
        perror("Execvp error");
    } else {
        if (!run_in_bg_flag) {
            int status;
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

        if (rd_output) {
            int rd_argc = 0;
        }

        if (myargv[0] == NULL) {
            continue;
        } else if (strcmp(myargv[0], "cd") == 0) {
            cd(myargv);
        } else if (strcmp(myargv[0], "pwd") == 0) {
            pwd();
        } else {
            // execvp with fork to not exit program
            execute(myargv, myargc);
        }
    }

    return 0;
}


////// TODO null terminate before < > << >>