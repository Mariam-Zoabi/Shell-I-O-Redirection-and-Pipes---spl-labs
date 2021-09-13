#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> //contains the getcwd func wich gets curr_set working directory
                    //+ the execl, execlp, execle, execv, execvp, execvpe funcs - execute a file
#include <wait.h>
#include <errno.h>
#include "LineParser.h"
#include "limits.h"
#include <sys/stat.h>   
#include <fcntl.h>      

#define MAX_INPUT_LINE 2048 

typedef struct map {
    char* name;
    char* value;
    struct map* next;
} map;

int debug_mode = 0;
map* set = NULL;



////////////////////
int execute(cmdLine *cmd_line){ 
    
    int cpid = -1;
    int status;
    int options = 0;


    if(strcmp("cd", cmd_line -> arguments[0]) == 0){
        if((chdir(cmd_line -> arguments[1])) == 0)
            printf("--cd command excuted sucssesfully--\n");
        else{
            fprintf(stderr ,"--cd command failed--\n");
            }
        }

        else if(strcmp("set", cmd_line -> arguments[0]) == 0){
       
        if(cmd_line->argCount < 3){    
            fprintf(stderr, "can't set [name] [value] from: %d elements\n", cmd_line->argCount);
            return 0;
        }

        if (set == NULL) {
                set = (map*)malloc(sizeof(map));
                set->name = strdup(cmd_line->arguments[1]);
                set->value = strdup(cmd_line->arguments[2]);
                set->next = NULL;
            }
        else{ // set != null
            map* curr_set = set;
            while(curr_set->next != NULL){//search if there is set with the same name
                if (strcmp(curr_set->name, cmd_line->arguments[1]) == 0) {
                    free(curr_set->value);///free the one that already exist
                    curr_set->value = (cmd_line->arguments[2]);///add the new one
                }
                else
                    curr_set = curr_set->next;  
            }

            if (strcmp(curr_set->name, cmd_line->arguments[1]) == 0) {///since we dont check the last one(curr_set->next)
                free(curr_set->value);
                curr_set->value = strdup(cmd_line->arguments[2]);
            }
            else{
                curr_set->next = (map*)malloc(sizeof(map));
                curr_set = curr_set -> next;
                curr_set->name = strdup(cmd_line->arguments[1]);
                curr_set->value = strdup(cmd_line->arguments[2]);
                curr_set->next = NULL;                            
            }           
        }

    }
    else if(strcmp("vars", cmd_line->arguments[0]) == 0) {
            printf("Name\tValue\n");
            map* curr_set = NULL;
            curr_set = set;
            while (curr_set != NULL) {
                printf("%s\t%s\n", curr_set->name, curr_set->value);
                curr_set = curr_set->next;
            }
        }
                
       //printf("%s %s \n\n", set->name, set->value);
           

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

            if(execvp(cmd_line -> arguments[0],cmd_line -> arguments)==-1)//////////////////////////////////////////
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
    return 0;
}


int activateInternalVariables(cmdLine** cmd_line){
    cmdLine* curr_cmdLine = *cmd_line;
    map* curr_set = set;
    //int found = 0;
    while(curr_cmdLine != NULL){
        for(int i=0; i< curr_cmdLine->argCount; i++){
            if(curr_cmdLine->arguments[i][0] == '$'){
                while (curr_set!=NULL){
                    if(strcmp(curr_set->name, &(curr_cmdLine->arguments[i][1]))==0){
                        replaceCmdArg(curr_cmdLine,i,curr_set->value);
                        break;
                    }
                    else
                        curr_set = curr_set->next;
                }
                if(curr_set==NULL){
                    printf("could't find the suitable variable\n");
                }
                
            }
        }
    curr_cmdLine = curr_cmdLine->next;
    }
    return 0;
}


int main(int argc, char* argv[], char *envp[]){

    
    FILE* input_stream = stdin;
    char buff_line[MAX_INPUT_LINE];
    char curr_work_dir[PATH_MAX];          
    cmdLine* cmd_line = NULL;


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

    set = (map*) malloc(sizeof(map));
    set->name = strdup("~");
    set->value = getenv("HOME");
    set->next = NULL;

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

        if(activateInternalVariables(&cmd_line)==0)
        execute(cmd_line);

        freeCmdLines(cmd_line);
        cmd_line = NULL;
    }
    return 0;
}