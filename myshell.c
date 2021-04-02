//shell function
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> 
#include <pwd.h>
#include <limits.h>   
#include <sys/wait.h>
#include <dirent.h>
#include "myshell.h"

cmdList cmdL;    //list of commands

int main(int argc, char **argv){
    char shell_path[STRINGSIZE] = "shell= ";
    strcat(shell_path, getenv("PWD"));
    //strcat add the source to destination
    strcat(shell_path,"/myshell");
    putenv(shell_path); //add the working directory 
    setenv("PATH","/bin",1);// add the current working directory
    
    //Depends on user input will go to either interactive or batch mode
    if (argc == 1)
        
        InteractiveMode();
    else if (argc == 2)
        BatchMode(argv[1]);
    else{
        printf("Error\n");
    }

    return EXIT_SUCCESS;
}

void BatchMode(char *filename){
    printf("myshell is running... \n");
    FILE * fInput = fopen(filename, "r");
    int isValidRead = 0;
    if (fInput == NULL){
        printf("Error\n");
        return;
    }
    while (isValidRead != -1){
        memset(&cmdL,0x00, sizeof(cmdL));
        //get user input
        char *command = GetCmd(&isValidRead, fInput);
        if (command == NULL){
            continue;
        }
        if (isValidRead == -1){
            free(command);
            continue;
        }
        char **cmdArgs = lArray(command);
        if (*cmdArgs == NULL){
            continue;
        }

        SplitCommands(cmdArgs, &cmdL);
        OrgCommands(&cmdL);
        myshell(&cmdL);
        free(cmdArgs);
        freeArray(&cmdL);
        free(command);

    }
    fclose(fInput);
    printf("-----myshell end----- \n");
}//batch mode

void InteractiveMode(){
    int isValidRead = 0; 
    while (1){
        prompt();
        memset(&cmdL,0x00, sizeof(cmdL));
        char *command = GetCmd(&isValidRead, stdin);//takes in standard user input
        if (command == NULL || isValidRead == -1 ) {continue;}
        if (command[0]=='\033' || command[0]==32) {continue;}
        char **cmdArgs = lArray(command);  //command line to commands
        if (*cmdArgs == NULL){continue;}
        
        
        //Split commands if more than one command 
        SplitCommands(cmdArgs, &cmdL);
        //Organize commands
        OrgCommands(&cmdL);
        //Run the commands
        myshell(&cmdL);

        // free resource
        free(cmdArgs);
        freeArray(&cmdL);
        free(command);
    }
}// Interactive mode

void prompt(){
    char userName[50] = {0};
    char hostname[50] = {0};
    char address[250] = {0};
    getUsername(userName, 50); 
    getHostname(hostname, 50);
    getcwd(address, 250);
    printf("%s@%s:%s$ ", userName, hostname,address);
    printf("%s> ", "myshell");
}//user prompt

int myshell(cmdList *cmdList)
{
    if (cmdList == NULL){
        return 1;
    }


    if (cmdList->iCmdTotal == 1){
        
        if (cmdList->pCommand[0].args[0] == NULL){
            return 1;
        }

        //check for internal command
        if (internalCmd(&cmdList->pCommand[0]) != -1){return EXIT_SUCCESS;}
    }

    return externalCmd(cmdList);//else run as external command
}

// run all commands in a command list or pipe
int externalCmd(cmdList *cmdList){
    if (cmdList->iCmdTotal == 1){
        //run a command
        RCommand(&cmdList->pCommand[0]);
        
    }
    else if (cmdList->iCmdTotal > 1 && cmdList->isPipe != 1){
        // run commands one by one
        for (int i = 0; i < cmdList->iCmdTotal; i++)
            RCommand(&cmdList->pCommand[i]);
    }
    else{
        // run commands with pipe handling
        cmdPipe(cmdList);
    }
    return EXIT_SUCCESS;
}

//Run a command
int RCommand(cmdString *cmd){
    pid_t pid;
    int status;
    pid = fork();

    
    char max_file_path[100] = {0};
    char parent[100] = {0};
    strcpy(parent,"parent=") ;
    strcat(parent, getenv("shell"));
    FILE * file=NULL,* inFileFp;
    if (cmd->isRedirOutputFile){
        //redirected output file
        GetAbsPath(max_file_path, cmd->sOutputFile);
        file=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    if (pid == 0){
        // The Child Process
        // Set the enviornment for the child process
        putenv( parent);
        printf("%s, %s", cmd->args[0], cmd->args[1]);

        //check if need open file
        if (cmd->isRedirInputFile){
            GetAbsPath(max_file_path,cmd->sInputFile);
            inFileFp= freopen(max_file_path,"r",stdin); // open file
        }
        //run command
        if (execvp(cmd->args[0], cmd->args) == -1){
            if (strcmp(cmd->args[0],"wc")==0){
                char* args[] = {"wc",cmd->args[1],NULL};
                execvp("/usr/bin/wc",args);
            }
            if (internalCmd(cmd)!=-1){return 1;}
            exit(EXIT_FAILURE);
        }

        //close file
        if (inFileFp)
            fclose(inFileFp);

    }
    else if (pid < 0){
        printf("Error\n");
    }
    else{
        if (!cmd->isBackgroundRunning){
            do
            {
                (void)waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        if(file){
            fclose(file);
            freopen("/dev/tty","w",stdout);
        }
    }
    return 1;
}

int cmdPipe(cmdList *cmdList){
        exit(EXIT_FAILURE);
}

int internalCmd(cmdString *cmd){
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

//buildin function - CD
int Process_CD (cmdString *cmd){
    int argsCount = CountArg(cmd->args);

    if (argsCount  > 1){
        printf("Error\n");
        return 1;
    }
    else if (argsCount == 0){
        char cwd[STRINGSIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
    }
    else{
        if (chdir(cmd->args[argsCount])){
            printf("Error\n");
            printf("\n");
            return 1;
        }
    }
    return EXIT_SUCCESS;
}


int Process_CLR (){
    printf("\033[H\033[J");
    return EXIT_SUCCESS;
}//clear the screen

int Process_DIR (cmdString *cmd){
    FILE * file=NULL;
    int argsCount = CountArg(cmd->args);
    struct dirent *de;  //directory entry
    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        file=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }
    DIR *dr;
    if (argsCount == 0){
        dr = opendir(".");
    }
    else{
        dr = opendir(cmd->args[argsCount]);
    }
    if (dr == NULL){
        printf("Error\n");
        return 1;
    }
    while ((de = readdir(dr)) != NULL){printf("%s\n", de->d_name);}
    closedir(dr);
    if(file){
        fclose(file);
        freopen("/dev/tty","w",stdout);
    }
    return EXIT_SUCCESS;
}//ls command

//buildin function - enviorn
int Process_ENVIORN (cmdString *cmd){
    FILE * file=NULL;
    int i = 0;

    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        file=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }

    while(__environ[i]) {
        printf("%s\n", __environ[i++]); 
    }

    if(file){
        fclose(file);
        freopen("/dev/tty","w",stdout);
    }
    return EXIT_SUCCESS;
}

int Process_ECHO (cmdString *cmd){
    int argsCount;
    FILE * file=NULL;
    if (cmd->isRedirOutputFile){
        char max_file_path[100] = {0};
        GetAbsPath(max_file_path, cmd->sOutputFile);
        file=freopen(max_file_path, cmd->isOutputTruncated ==  1 ? "w": "a",stdout);
    }
    for (argsCount = 1; ((argsCount < MAXARGS) && (cmd->args[argsCount] != NULL)); argsCount++){
        printf("%s \t",cmd->args[argsCount]);
    }
    printf("\n");
    if(file){
        fclose(file);
        freopen("/dev/tty","w",stdout);
    }
    return 1;
}//echo

int Process_HELP (cmdString *cmd){
}//help

int Process_PAUSE (){
    getpass("Paused. Press 'ENTER' to continue.");
    return EXIT_SUCCESS;
}//pause program

int Process_QUIT (){
    exit(0);
}//exit program
