#define PROMPT_SHELL "shell"
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

void Prompt();
void IM();