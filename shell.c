#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/wait.h>
#include <dirent.h>
#include "shell.h"

void Prompt(){
    char userName[100] = {0};
    char hostname[100] = {0};
    char currentPath[250] = {0};

    getUsername(userName, 100);
    getHostname(hostname, 100);
    getcwd(currentPath, 250);

    printf("%s@%s:%s$ ", userName, hostname,currentPath);
    printf("%s>", PROMPT_MYSHELL);
}

void IM(){
    int isValidRead = 0;
    signal(SIGINT, sigintHandler);

    while (1){
        Prompt();
        memset(&cmdList,0x00, sizeof(cmdList));

        char *command = getCmdLine(&isValidRead, stdin);

        if (command == NULL || isValidRead == -1 ) {continue;}
        if (command[0]=='\033' || command[0]==32) {continue;}

        char **cmdArgs = LineToArray(command);
        if (*cmdArgs == NULL){
            continue;
        }


        SplitCommands(cmdArgs, &cmdList);
        OrgCommands(&cmdList);
        RunMyShell(&cmdList);

        free(cmdArgs);
        FreeCommandArray(&cmdList);
        free(command);
    }
}

void BatchMode(char *filename){
    printf("myshell is running... \n");

    FILE * fInput = fopen(filename, "r");
    int isValidRead = 0;

    if (fInput == NULL){
        ShowError();
        printf("\n");
        return;
    }

    while (isValidRead != -1){
        memset(&cmdList,0x00, sizeof(cmdList));

        char *command = getCmdLine(&isValidRead, fInput);
        if (command == NULL){
            continue;
        }
        if (isValidRead == -1){
            free(command);
            continue;
        }
        char **cmdArgs = LineToArray(command);
        if (*cmdArgs == NULL){
            continue;
        }


        SplitCommands(cmdArgs, &cmdList);
        OrgCommands(&cmdList);
        RunMyShell(&cmdList);

        free(cmdArgs);
        FreeCommandArray(&cmdList);
        free(command);

    }
    fclose(fInput);
    printf("-----myshell end----- \n");
}

int RunMyShell(StrCmdArray *cmdList)
{
    if (cmdList == NULL){
        return 1;
    }


    if (cmdList->iCmdTotal == 1){

        if (cmdList->pCommand[0].args[0] == NULL){
            return 1;
        }

        if (RunBuildInCmd(&cmdList->pCommand[0]) != -1){return EXIT_SUCCESS;}
    }

    return RunCommands(cmdList);
}

int RunCommands(StrCmdArray *cmdList){
    if (cmdList->iCmdTotal == 1){
        RunACommand(&cmdList->pCommand[0]);

    }
    else if (cmdList->iCmdTotal > 1 && cmdList->isPipe != 1){
        for (int i = 0; i < cmdList->iCmdTotal; i++)
            RunACommand(&cmdList->pCommand[i]);
    }
    else{
        RunCommandsPipe(cmdList);
    }
    return EXIT_SUCCESS;
}