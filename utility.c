#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> 
#include <pwd.h>
#include "myshell.h"
void getUsername(char *stmp, int size) {
    struct passwd* pwd = getpwuid(getuid());
    strncpy(stmp, pwd->pw_name, size);
}//return name of the user


void getHostname(char *stmp, int size) {
    gethostname(stmp, size);
}//return name of the system host

char *GetCmd(int *isvalid, FILE* inputStream) {
    char *cmd = (char *)malloc(sizeof(char) * STRINGSIZE);
    memset(cmd,  0x00, sizeof(char) * STRINGSIZE);

    size_t len = STRINGSIZE;
    *isvalid= getline(&cmd, &len, inputStream);

    if(cmd[ strlen(cmd) - 1] == '\n') {
        cmd[ strlen(cmd) - 1] = '\0';
    }

    return cmd;
}//Read user command

char** lArray(char *line){
    char **argumentList = (char **)malloc(sizeof(char *) * MAXARGS);
    memset(argumentList,0x00, sizeof(char *) * MAXARGS);
    if (argumentList == NULL){
        printf("Error\n");
        return NULL;
    }

    char *arg;
    char Tmp[50];
    char delim[10] = " \t\n\r\a";
    int pos = 0;
    
    strcpy(Tmp,line);

    arg = strtok(line, delim);

    while (arg != NULL && pos < MAXARGS){
        argumentList[pos] = arg;
        pos ++;
        arg = strtok(NULL, delim);
    }

    argumentList[pos] = NULL;
    
    
    return argumentList;
}//parse command

void SplitCommands(char**args, cmdList *cmdList){
    if (args == NULL || *args == NULL || cmdList == NULL){
        printf("Error\n");
        return;
    }

    int index;
    int count = 0u;
    int argumentCnt = 0u;

    //split from here
    for (index = 0; index < MAXARGS && args[index] != NULL; index++){
        if (cmdList->pCommand[count].args == NULL){
            cmdList->pCommand[count].args = (char **)malloc(sizeof(char *) * MAXARGS);
            memset(cmdList->pCommand[count].args, 0x00, sizeof(char *) * MAXARGS);
            argumentCnt = 0;
        }
        
        if ((strcmp(args[index], "&") != 0 &&  strcmp(args[index], "|") != 0) || (strcmp(args[index], "&") == 0 &&  args[index+1] == NULL)){
            cmdList->pCommand[count].args[argumentCnt] = args[index];
            argumentCnt++;
        }
        else { 
            cmdList->pCommand[count].args[argumentCnt] = NULL;
            if (strcmp(args[index], "|") == 0)
            {
                cmdList->isPipe = 1;
            }
            count++;
        }
    }
    //number of command
    cmdList->iCmdTotal = count+1;
}//split the commands


void OrgCommands(cmdList *cmdList){
    if (cmdList == NULL){
        printf("Error\n");
        return;
    }
    //read all the commands limit is 20
    for (int j = 0; j < 20; j++){
        StrCmd *cmd = &cmdList->pCommand[j];
        if (cmd->args == NULL ){
            return;
        }

        
         // handle > >> < ; | 
        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if (strcmp(cmd->args[i], "<") ==0 || strcmp(cmd->args[i], ">") == 0 \
                || strcmp(cmd->args[i], ">>") == 0|| strcmp(cmd->args[i], "|") ==0)
            {
                if ((cmd->args[i+1] == NULL) || (strcmp(cmd->args[i+1], "<") == 0|| strcmp(cmd->args[i+1], ">") == 0 \
                        || strcmp(cmd->args[i+1], ">>") == 0|| strcmp(cmd->args[i+1], "|") ==0))
                {
                    printf("Error\n");
                    return;
                }
            }
        }

        // handle input file with redirect
        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if (strcmp(cmd->args[i], "<") == 0 && cmd->args[i+1] != NULL )
            {
                cmd->sInputFile = cmd->args[i+1];
                char buff[FILENAME_MAX];
                getcwd( buff, FILENAME_MAX );
                cmd->isRedirInputFile = 1;
            }
        }

        // handle output file with redirect
        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if (((strcmp(cmd->args[i], ">") == 0) || (strcmp(cmd->args[i], ">>") == 0)) && (cmd->args[i+1] != NULL ))
            {
                char buff[FILENAME_MAX];
                getcwd( buff, FILENAME_MAX );
                strcat(buff,"/");
                cmd->sOutputFile = cmd->args[i+1];
                cmd->isRedirOutputFile = 1;
                
                if (strcmp(cmd->args[i], ">>") == 0){
                    cmd->isOutputTruncated = 1;
                }
            }
        }

        //Check if the pipe is used
        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if (((strcmp(cmd->args[i], "|") == 0)) && (cmd->args[i+1] != NULL ))
            {
                cmdList->isPipe = 1;
            }
        }

        //Check if the command is to be run in background
        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if (strcmp(cmd->args[i], "&") == 0)
            {
                cmd->isBackgroundRunning = 1;
            }
        }

        for (int i = 0; i < MAXARGS && cmd->args[i] != NULL; i++){
            if(strcmp(cmd->args[i], "<") ==0 || strcmp(cmd->args[i], ">") == 0 \
                || strcmp(cmd->args[i], ">>") == 0|| strcmp(cmd->args[i], "|") == 0 ||  strcmp(cmd->args[i], "&") == 0)
            {
                for (int k = i; k < MAXARGS; k++)
                {
                    cmd->args[k] = NULL;
                }
                break;
            }
        }
    }
}

int CountArg(char *const *args){
    int argsCount;
    for (argsCount = 0; ((argsCount < MAXARGS) && (args[argsCount+1] != NULL)); argsCount++){

    }
    return argsCount;
}

//get a absolute path from input path
void GetAbsPath(char *AbsolutePath,const char *CurPath) {
    char *old_dir, *current_dir;
    int i,j;
    i=j=0;
    AbsolutePath[0]=0;
    if(CurPath[0]=='~'){
        printf("~\n");
        strcpy(AbsolutePath, getenv("HOME"));
        j=strlen(AbsolutePath);
        i=1;
    }
    else  if(CurPath[0]=='.'&&CurPath[1]=='.'){
        old_dir=(char *)malloc(1024);
        getcwd(old_dir,1024);

        chdir("..");
        current_dir=(char *)malloc(1024);
        getcwd(current_dir,1024);
        strcpy(AbsolutePath, current_dir);
        j=strlen(AbsolutePath);
        i=2;
        chdir(old_dir);
    }
    else   if(CurPath[0]=='.'){
        getcwd( AbsolutePath, 1024);
        j=strlen(AbsolutePath);
        i=1;
    }
    else if(CurPath[0]!='/'){
        getcwd( AbsolutePath, 1024);
        strcat(AbsolutePath, "/");
        j=strlen(AbsolutePath);
        i=0;
    }
    strcat(AbsolutePath+j,CurPath+i);
}

void freeArray(cmdList *cmdList){
    for (int i = 0; i < 20; i++){
        StrCmd *cmd = &cmdList->pCommand[i];
        if (cmd){
            free(cmd->args);
        }
    }
}




