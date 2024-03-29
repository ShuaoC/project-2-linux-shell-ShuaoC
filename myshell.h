#define STRINGSIZE 1024
#define MAX_ARG_LENGTH 64
#define MAXARGS 64

typedef struct cmdString{
    int iCmdID;
    int iCmdType;
    char* sInputFile;
    char* sOutputFile;
    char** args;
    int isRedirInputFile;
    int isRedirOutputFile;
    int isBackgroundRunning;
    int isOutputTruncated;
} cmdString;

typedef struct cmdList{
    int iCmdTotal;
    int isPipe;
    cmdString pCommand[20];
}cmdList;

int myshell(cmdList *cmdList);
void prompt();
void InteractiveMode();
void BatchMode(char *filename);
int Process_CD (cmdString *cmd);
int Process_CLR ();
int Process_DIR (cmdString *cmd);
int Process_ENVIORN (cmdString *cmd);
int Process_ECHO (cmdString *cmd);
int Process_HELP (cmdString *cmd);
int Process_PAUSE ();
int Process_QUIT ();
int Process_test();
void getUsername(char*username, int size);
void getHostname(char *hostname, int size);
int CountArg(char *const *args);
void GetAbsPath(char *AbsolutePath, const char *CurPath);
int CountArg(char *const *args);
void freeArray(cmdList *cmdList);
char *GetCmd( int *isvalid, FILE* inputStream );
char** lArray(char *line);
void OrgCommands(cmdList *cmdList);
void SplitCommands(char**args, cmdList *cmdList);
int internalCmd(cmdString *cmd);
int externalCmd(cmdList *cmdList);
int RCommand(cmdString *cmd);
int RunCommandsNotWait(cmdList *cmdList);
int cmdPipe(cmdList *cmdList);



 


