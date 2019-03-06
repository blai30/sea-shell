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

void clear() {
    printf("\033[H\033[J");
}

void print_dir() {
    char cur_dir[1024];
    getcwd(cur_dir, sizeof(cur_dir));

    printf(PROMPT, cur_dir);
}

void execute(char** myargv, int myargc) {
    // If no command was entered
    if (myargv[0] == NULL) {
        return;
    }

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

    if (strcmp(myargv[0], "cd") == 0) {
        if (!myargv[1]) {
            perror("Enter a directory");
        } else {
            chdir(myargv[1]);
        }
    } else if (strcmp(myargv[0], "pwd") == 0) {
        char pwd[1024];
        printf("%s\n", getcwd(pwd, sizeof(pwd)));
    } else {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork error: fork() < 0");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execvp(myargv[0], myargv);
            perror("Execvp error");
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

// Parse buffer and count arguments
char** parse_buffer(char* buf, int *arg_c) {
    char** tokens = malloc(sizeof(char*) * 4);
    char* token = strtok(buf, " \n\t\r\v\f");
    int count = 0;
    while (token != NULL) {
        count++;
        token = strtok(NULL, " \n\t\r\v\f");
    }
    tokens[count] = NULL;  // NULL terminate array
    *arg_c = count;

    return tokens;
}

// the project came as int* argc but Souza confirmed it should be int argc
int main(int argc, char** argv) {

    char buffer[BUFFERSIZE];

    clear();

    while (1) {
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

        printf("myargc = %d\n", myargc);

        // execvp with fork to not exit program
        execute(myargv, myargc);
    }

    return 0;
}


////// null terminate before < > << >>