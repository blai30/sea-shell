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
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

int main(int* argc, char** argv) {

    char user_input[BUFFERSIZE];
    char *myargv[4];
    int myargc;

    printf(PROMPT);
    scanf("%s", user_input);
    printf("%s", user_input);

    int i = 0;
    myargv[i] = strtok(user_input, " ");
    while (myargv[i] != NULL) {
        myargv[++i] = strtok(user_input, " ");
    }

    for (int j = 0; j < sizeof(myargv); j++) {
        printf("%s", myargv[j]);
    }

    return 0;
}
