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

char** parse(char* line, int** argc) {
    char* token = strtok(line, " \n\t\r\v\f");
    char** tokens = malloc(sizeof(char*) * 4);

    int i = 0;
    while (token != NULL) {
        tokens[i] = token;
        i++;
        token = strtok(NULL, " \n\t\r\v\f");
    }

    tokens[i] = "\0";
    *argc = i;

    return tokens;
}

// the project came as int* argc but the compiler complains
int main(int* argc, char** argv) {

    char buffer[BUFFERSIZE];
    char** myargv;
    int* myargc = 0;

    clear();

    while (1) {
        print_dir();

        fgets(buffer, BUFFERSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';    // Clear \n

        printf("%s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            exit(0);
        }

        myargv = parse(buffer, &myargc);

        printf("%s\n", myargv[0]);
        printf("%d\n", myargc);
    }

    return 0;
}
