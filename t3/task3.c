#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> //contains the getcwd func wich gets current working directory
                    //+ the execl, execlp, execle, execv, execvp, execvpe funcs - execute a file
#include <wait.h>
#include <errno.h>
#include "LineParser.h"
#include "limits.h"

#include <fcntl.h>              /* Obtain O_* constant definitions */

#define MAX_BUFFER  80

int debug_mode = 0;


int main(int argc, char** argv){
     //The array pipefd is used to return two file descriptors referring to the ends of the pipe
    pid_t cpid1 = -1;
    pid_t cpid2 = -1;
    int pipefd[2];

    for(int i=0; i< argc; i++){
        if(strcmp("-d", argv[i]) == 0){
            debug_mode = 1;
            break;
        }
    }


    if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    if (debug_mode)
        fprintf(stderr, "Parent Process - before forking\n");
    

    cpid1 = fork();

    if (cpid1 == -1) {
        perror("fork error - 1");
        exit(EXIT_FAILURE);
    }

        /////child-1//////
    if (cpid1 == 0) {
        
        if(close(pipefd[0]) == -1){
            perror("child-1 could't close");
            _exit(EXIT_FAILURE);
        }        
        
        if(debug_mode)
            fprintf(stderr, "child1 redirecting stdout to the write end of the pipe…\n");
    
        
        if(close(STDOUT_FILENO) == -1){
            perror("child-1 could't close the standad output");
            _exit(EXIT_FAILURE);
        }

        if(dup(pipefd[1]) < 0 ){
            perror("child-1 could't duplicate");
            _exit(EXIT_FAILURE);    
        }

        if(close(pipefd[1]) < 0 ){
            perror("child-1 could't close the duplicated fd");
            _exit(EXIT_FAILURE);    
        }

        if(debug_mode)
            fprintf(stderr, "child1 going to execute cmd: …\n");

        char* cmd1[] = {"ls", "-l", NULL};
        if(execvp(cmd1[0], cmd1) == -1)
            perror("error in excuting 'ls -l'\n");       

        _exit(EXIT_SUCCESS);

    } else {
       //pipefd[0] for reading
       //pipefd[1] for writing

        /*
        if we don't close the write end of the pipe, 
        then the Read side of the pipe will NOT get EOF
        */
        if(close(pipefd[1])==-1){/////step-4: Close the write end of the pipe.
            perror("\n");
            exit(EXIT_FAILURE); 
        }
        
        if(debug_mode)//after forking
            fprintf(stderr, "parent_process>created process with id: %d\n", cpid2);
        
        if(debug_mode)//before closing
            fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
        
        
        cpid2 = fork();
        
        if(cpid2 == -1){
            perror("fork error - 2");
            exit(EXIT_FAILURE);
        }

        if(cpid2==0){////child2
            if(close(STDIN_FILENO) == -1){
                perror("child-2 could't close the standad input");
                _exit(EXIT_FAILURE);
            }

            if(dup(pipefd[0]) < 0 ){
                perror("child-2 could't duplicate");
                _exit(EXIT_FAILURE);    
            }

            if(close(pipefd[0]) < 0 ){
                perror("child-2 could't close the duplicated fd");
                _exit(EXIT_FAILURE);    
            }

            if(debug_mode)
                fprintf(stderr, "child2 going to execute cmd: …\n");

            char* cmd1[] = {"tail", "-n","2", NULL};//tail -n 2
            if(execvp(cmd1[0], cmd1) == -1)
                perror("error in excuting 'tail -n 2'\n");       

            _exit(EXIT_SUCCESS);
        }
        
        else{


            if(debug_mode)//after forking
                fprintf(stderr, "parent_process>created process with id: %d\n", cpid2);
             if(debug_mode)//before closing
                fprintf(stderr, "parent_process>closing the write end of the pipe…\n");

            /*
            if we don't close the read side of the pipe, nothing would change 
            */
            if(close(pipefd[0])==-1){/////step-7: Close the read end of the pipe.
                perror("\n");
                exit(EXIT_FAILURE); 
            }

            if(debug_mode)
                fprintf(stderr,"parent_process>waiting for child-1 processes to terminate…");

            if(waitpid(cpid1, NULL, 0 ) == -1)
                perror("error in waiting for child-1\n");

            if(debug_mode)
                fprintf(stderr,"parent_process>waiting for child-2 processes to terminate…");


            if(waitpid(cpid2, NULL, 0 ) == -1)
                perror("error in waiting for child-2\n");

        }           

        exit(EXIT_SUCCESS);
    }

}