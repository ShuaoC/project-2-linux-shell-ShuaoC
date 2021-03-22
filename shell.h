#define PROMPT_MYSHELL "shell"
#define MAX_STRINGSIZE 1024
#define MAX_ARG_LENGTH 64
#define MAX_NUM_OF_ARGUMENTS 64

typedef struct StrCmd{
    int iCmdID;
    int iCmdType;
    char* sInputFile;
    char* sOutputFile;
    char** args;
    int isRedirInputFile;
    int isRedirOutputFile;
    int isBackgroundRunning;
    int isOutputTruncated;
} StrCmd;

typedef struct StrCmdArray{
    int iCmdTotal;
    int isPipe;
    StrCmd pCommand[20];
}StrCmdArray;

int RunMyShell(StrCmdArray *cmdList);
void ShowPrompt();
void InteractiveMode();
void BatchMode(char *filename);

void sigintHandler(int sig_num);
void getUsername(char*username, int size);
void getHostname(char *hostname, int size);
int GetArgsCount(char *const *args);
void GetAbsPath(char *AbsolutePath, const char *CurPath);
int GetArgsCount(char *const *args);
void FreeCommandArray(StrCmdArray *cmdList);
void ShowError();

char *getCmdLine( int *isvalid, FILE* inputStream );
char** LineToArray(char *line);
void OrgCommands(StrCmdArray *cmdList);
void SplitCommands(char**args, StrCmdArray *cmdList);
int RunBuildInCmd(StrCmd *cmd);
int RunCommands(StrCmdArray *cmdList);
int RunACommand(StrCmd *cmd);
int RunCommandsNotWait(StrCmdArray *cmdList);
int RunCommandsPipe(StrCmdArray *cmdList);

int Process_CD (StrCmd *cmd);
int Process_CLR ();
int Process_DIR (StrCmd *cmd);
int Process_ENVIORN (StrCmd *cmd);
int Process_ECHO (StrCmd *cmd);
int Process_HELP (StrCmd *cmd);
int Process_PAUSE ();
int Process_QUIT ();
int Process_test();


