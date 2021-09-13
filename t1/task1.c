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
#include <sys/stat.h>   
#include <fcntl.h>      

#define MAX_INPUT_LINE 2048 


int execute(cmdLine *pCmdLine){
    return execvp(pCmdLine -> arguments[0],pCmdLine -> arguments); 
}


int main(int argc, char* argv[], char *envp[]){

    int cpid = -1;
    FILE* input_stream = stdin;
    char buff_line[MAX_INPUT_LINE];
    char curr_work_dir[PATH_MAX];          
    cmdLine* cmd_line = NULL;
    int debug_mode = 0;
    int status;
    int options = 0;

    //initializing the array's with 0
    for(int i=0; i<PATH_MAX; i++){
        curr_work_dir[i] = 0;
        if(i<MAX_INPUT_LINE){
            buff_line[i] = 0;
        }
    }

    for(int i=1; i<argc ; i++){
        if(strcmp("-d", argv[i]) == 0){
            debug_mode = 1;
            break;
        }
    }

    if(debug_mode==1)
        fprintf(stderr,"debug mode is ON\nParent prossec ID: %d\n",getpid());

        //---------------------THE INFINIT LOOP------------------------
    while(true){

        getcwd(curr_work_dir,PATH_MAX); 
        printf("the current working directory is %s\n", curr_work_dir);
        
        fgets(buff_line, MAX_INPUT_LINE, input_stream); 
        if(feof(input_stream) || (strcmp(buff_line,"quit\n") == 0))
            break;
    
        cmd_line = parseCmdLines(buff_line);

    
        for(int i=1; i< cmd_line -> argCount ; i++){
            if(strcmp("cd", cmd_line -> arguments[0]) == 0){
                if((chdir(cmd_line -> arguments[1])) == 0)
                    printf("--cd command excuted sucssesfully--\n");
                else{
                    fprintf(stderr ,"--cd command failed--\n");
                    }
            }
        }
        //------- here is the fork--------
        cpid = fork();
        
        if(cpid < 0){//fork error
                perror("Fork error");
            exit(EXIT_FAILURE);
        }
        
        if(cpid == 0){//child
            
            if(debug_mode == 1){
                fprintf(stderr,"The Executing command: %s\n\n" ,cmd_line -> arguments[1]);
            }

            if(cmd_line -> inputRedirect != NULL){

                int input_redirect = open(cmd_line -> inputRedirect,O_RDONLY);

                if(input_redirect == -1){//we could open the fd
                    perror("error in opening the fd");
                    _exit(EXIT_FAILURE);
                }
                
                if(close(STDIN_FILENO) == -1){
                    perror("error in closing the stdin fd\n");
                    _exit(EXIT_FAILURE);
                }

                if(dup(input_redirect) == -1){
                    perror("error in duplicatin the fd\n");
                    _exit(EXIT_FAILURE);
                }

                if(close(input_redirect) == -1){
                    perror("error in closing the fd\n");
                    _exit(EXIT_FAILURE);
                }
            }

            if(cmd_line -> outputRedirect != NULL){

                int output_redirect = open(cmd_line -> outputRedirect, O_CREAT | O_WRONLY | O_APPEND, 0777); // 0777 octal

                if(output_redirect == -1){//we could't open the fd
                    perror("error in opening the fd");
                    _exit(EXIT_FAILURE);
                }
                
                if(close(STDOUT_FILENO) == -1){
                    perror("error in closing the stdout fd\n");
                    _exit(EXIT_FAILURE);
                }

                if(dup(output_redirect) == -1){
                    perror("error in duplicatin the fd\n");
                    _exit(EXIT_FAILURE);
                }

                if(close(output_redirect) == -1){
                    perror("error in closing the fd\n");
                    _exit(EXIT_FAILURE);
                }
            }

            if(execute(cmd_line) == -1)
                _exit(EXIT_FAILURE);

            freeCmdLines(cmd_line);
            _exit(EXIT_SUCCESS);
        }

        if(cpid > 0){//the parent 

            if(debug_mode == 1){
                fprintf(stderr,"The child prossec ID: %d\n" ,cpid);
            }
        
            if(cmd_line -> blocking == 1){
                if(waitpid(cpid, &status, options) == -1){
                    perror("waitpid error\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            //exit(EXIT_SUCCESS);
        } 
        freeCmdLines(cmd_line);
        cmd_line = NULL;
    }
    return 0;
}