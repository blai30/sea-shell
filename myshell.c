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

void execute(char** myargv) {
    if (strcmp(myargv[0], "cd") == 0) {
        if (!myargv[1]) {
            perror("Enter a directory");
        }
        chdir(myargv[1]);
    } else if (strcmp(myargv[0], "pwd") == 0) {
        char pwd[1024];
        printf("%s\n", getcwd(pwd, sizeof(pwd)));
    } else {
        pid_t pid = fork();
        int status;

        if (pid < 0) {
            perror("Fork error: fork() < 0");
            exit(1);
        } else if (pid == 0) {
            execvp(myargv[0], myargv);
            perror("Execvp error");
        } else {
            // need to add a case with &
            while (wait(&status) != pid) { ;
            }
        }
    }
}

// the project came as int* argc but Souza confirmed it should be int argc
int main(int argc, char** argv) {

    char buffer[BUFFERSIZE];

    clear();

    while (1) {
        char** myargv = malloc(sizeof(char*) * 4);
        int myargc = 0;

        print_dir();

        // Read line of input
        fgets(buffer, BUFFERSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;    // Clear \n

        printf("%s\n", buffer);

        // End program
        if (strcmp(buffer, "exit") == 0) {
            exit(0);
        }

        // Parse buffer and count arguments
        char* token = strtok(buffer, " \n\t\r\v\f");
        while (token != NULL) {
            myargv[myargc] = token;
            myargc++;
            token = strtok(NULL, " \n\t\r\v\f");
        }
        myargv[myargc] = NULL;  // NULL terminate array

        // execvp with fork to not exit program
        execute(myargv);

//        printf("%s\n", myargv[0]);
//        printf("%s\n", myargv[1]);
//        printf("%d\n", myargc);
    }

    return 0;
}
