#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/wait.h>
#include <dirent.h>
#include "shell.h"

StrCmdArray cmdList;

void ShowPrompt(){
    char userName[100] = {0};
    char hostname[100] = {0};
    char currentPath[250] = {0};

    getUsername(userName, 100);
    getHostname(hostname, 100);
    getcwd(currentPath, 250);

    printf("%s@%s:%s$ ", userName, hostname,currentPath);
    printf("%s>", PROMPT_MYSHELL);
}

void InteractiveMode(){
    int isValidRead = 0;
    signal(SIGINT, sigintHandler);

    while (1){
        ShowPrompt();
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

int RunBuildInCmd(StrCmd *cmd){

    if (strcmp(cmd->args[0],"cd")==0) {return Process_CD(cmd);}
    else if (strcmp(cmd->args[0],"clr")==0) {return Process_CLR();}
    else if (strcmp(cmd->args[0],"dir")==0) {return Process_DIR(cmd);}
    else if (strcmp(cmd->args[0],"environ")==0) {return Process_ENVIORN(cmd);}
    else if (strcmp(cmd->args[0],"echo")==0) {return Process_ECHO(cmd);}
    else if (strcmp(cmd->args[0],"help")==0) {return Process_HELP(cmd);}
    else if (strcmp(cmd->args[0],"pause")==0) {return Process_PAUSE();}
    else if (strcmp(cmd->args[0],"quit")==0) {return Process_QUIT();}
    else{return -1;}
}

int RunACommand(StrCmd *cmd){
    pid_t pid;
    int status;
    pid = fork();


    char max_file_path[100] = {0};
    char parent[100] = {0};
    strcpy(parent,"parent=") ;
    strcat(parent, getenv("shell"));
    FILE * outFileFp=NULL,* inFileFp;
    if (cmd->isRedirOutputFile){
        GetAbsPath(max_file_path, cmd->sOutputFile);
        outFileFp=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    if (pid == 0){
        putenv( parent);
        printf("%s, %s", cmd->args[0], cmd->args[1]);

        if (cmd->isRedirInputFile){
            GetAbsPath(max_file_path,cmd->sInputFile);
            inFileFp= freopen(max_file_path,"r",stdin);
        }
        if (execvp(cmd->args[0], cmd->args) == -1){
            if (strcmp(cmd->args[0],"wc")==0){
                char* args[] = {"wc",cmd->args[1],NULL};
                execvp("/usr/bin/wc",args);
            }
            if (RunBuildInCmd(cmd)!=-1){return 1;}
            exit(EXIT_FAILURE);
        }

        if (inFileFp)
            fclose(inFileFp);

    }
    else if (pid < 0){
        ShowError();
    }
    else{
        if (!cmd->isBackgroundRunning){
            do
            {
                (void)waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        if(outFileFp){
            fclose(outFileFp);
            freopen("/dev/tty","w",stdout);
        }
    }
    return 1;
}

int RunCommandsPipe(StrCmdArray *cmdList){
        size_t i, n;
        int prev_pipe, pfds[2];
        n = cmdList->iCmdTotal;
        prev_pipe = STDIN_FILENO;

        for (i = 0; i < n - 1; i++){
            pipe(pfds);
            if (fork() == 0)
            {
                if (prev_pipe != STDIN_FILENO) {
                    dup2(prev_pipe, STDIN_FILENO);
                    close(prev_pipe);
                }

                dup2(pfds[1], STDOUT_FILENO);
                close(pfds[1]);

                execvp(cmdList->pCommand[i].args[0], cmdList->pCommand[i].args);
                ShowError();
                exit(EXIT_FAILURE);
            }

            close(prev_pipe);
            close(pfds[1]);
            prev_pipe = pfds[0];
        }

        if (prev_pipe != STDIN_FILENO) {
            dup2(prev_pipe, STDIN_FILENO);
            close(prev_pipe);
        }

        execvp(cmdList->pCommand[i].args[0], cmdList->pCommand[i].args);
        ShowError();
        exit(EXIT_FAILURE);
}

int Process_CD (StrCmd *cmd){
    int num_of_args = GetArgsCount(cmd->args);

    if (num_of_args  > 1){
        ShowError();
        return 1;
    }
    else if (num_of_args == 0){
        char cwd[MAX_STRINGSIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
    }
    else{
        if (chdir(cmd->args[num_of_args])){
            ShowError();
            printf("\n");
            return 1;
        }
    }
    return EXIT_SUCCESS;
}

int Process_CLR (){
    printf("\033[H\033[J");
    return EXIT_SUCCESS;
}

int Process_DIR (StrCmd *cmd){
    FILE * outFileFp=NULL;
    int num_of_args = GetArgsCount(cmd->args);

    struct dirent *de;

    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        outFileFp=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    DIR *dr;
    if (num_of_args == 0){
        dr = opendir(".");
    }
    else{
        dr = opendir(cmd->args[num_of_args]);
    }

    if (dr == NULL){
        ShowError();
        return 1;
    }

    while ((de = readdir(dr)) != NULL)
        printf("%s\n", de->d_name);

    closedir(dr);

    if(outFileFp){
        fclose(outFileFp);
        freopen("/dev/tty","w",stdout);
    }

    return EXIT_SUCCESS;
}

int Process_ENVIORN (StrCmd *cmd){
    FILE * outFileFp=NULL;
    int i = 0;

    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        outFileFp=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    while(__environ[i]) {
        printf("%s\n", __environ[i++]);
    }

    if(outFileFp){
        fclose(outFileFp);
        freopen("/dev/tty","w",stdout);
    }
    return EXIT_SUCCESS;
}

int Process_ECHO (StrCmd *cmd){
    int num_of_args;
    FILE * outFileFp=NULL;

    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        outFileFp=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    for (num_of_args = 1; ((num_of_args < MAX_NUM_OF_ARGUMENTS) && (cmd->args[num_of_args] != NULL)); num_of_args++){
        printf("%s \t",cmd->args[num_of_args]);
    }


    printf("\n");

    if(outFileFp){
        fclose(outFileFp);
        freopen("/dev/tty","w",stdout);
    }
    return 1;
}

int Process_HELP (StrCmd *cmd){
    FILE * outFileFp=NULL;
    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        outFileFp=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    if (fork() == 0){
        char * const help[] = { "more", "readme", NULL };
        if (execvp(help[0], help) == -1){
            exit(EXIT_FAILURE);
        }
    }
    if(outFileFp){
        fclose(outFileFp);
        freopen("/dev/tty","w",stdout);
    }

    return EXIT_SUCCESS;
}

int Process_PAUSE (){
    getpass(" Myshell is paused. \n Press <ENTER> key to continue.");
    return EXIT_SUCCESS;
}

int Process_QUIT (){
    exit(0);
}

int main(int argc, char **argv){
    char * defaultPath = "/bin";
    char shell_path[MAX_STRINGSIZE]="shell=";
    strcat(shell_path, getenv("PWD"));
    strcat(shell_path,"/myshell");
    putenv(shell_path);
    setenv("PATH",defaultPath,1);
    if (argc == 1)

        InteractiveMode();
    else if (argc == 2)
        BatchMode(argv[1]);
    else
        ShowError();

    return EXIT_SUCCESS;
}
