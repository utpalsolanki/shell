/* --------------------------------------------------------------------- 
 * header: shell.h
 * Detail: header file. 
 * 		   Struct definition.
 * 		   Link list architecture.
 * --------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#define TRUE 		1
#define FALSE		0

// Max arguments user can give in command line
#define MAX_ARGS 	20
#define MAX_COMMAND_STRING		80
#define COMMAND_INPUT_CAPACITY	256

//Default commad acknowledgement
#define DEFAULT_SHELL_STRING	"shell~:"

//Max last history to display
#define MAX_COMMAND_IN_HISTORY	10

//Defualt logging file name
#define DEFAULT_SHELL_LOG		"myshell.log"
#define DEFAULE_RC_FILE 		".cs543rc"

#define VALIDATE_SUCCESS(x,y) if(x == NULL){printf("%s\n",y);return;}

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define NRM  "\x1B[0m"

//Function declaration
void verPrint(char *data);
void stopLogging(char *cmd[]);
void startLogging(char *cmd[]);
void commandHistory(char *cmd[]);
void processCommand(char *cmdOrig, char *cmdString, char *cmd[], int isWait);
void pushHistory(char *cmdOrig);
void pushAlias(char *cmdMain);
int isCommandAliased(char *cmdOrig, char *readString, char *cmd[]);
void setHandler(char *cmdOrig, char *readString, char *cmd[]);
void setEnvPath(char *cmdOrig, char *readString, char *cmd[]);
void setVerboseEnv(char *cmdOrig, char *readString, char *cmd[]);
int coreParser(char *cmdOrig, char *readString, char *command, char *args, char *cmd[]);
void checkRcFile(char *fileName, char *cmdOrig, char *readString, char *command, char *args, char *cmd[]);
void intermediatePipingCheck(char *cmdOrig, char *readString, char *command, char *args, char *cmd[]);
enum inputType parseInput(const char *inString, char **comm, char **args, char *cmd[]);

//State machine enums
enum inputType {
	INPUT_ERROR,
	NO_COMMAND,
	ONLY_COMMAND,
	ARGS_AND_COMMAND,
	ARGS_AND_COMMAND_DONT_WAIT
};

//List for Alias
struct aliasContainer_t {
	char aliasBase[COMMAND_INPUT_CAPACITY];
	char cmdString[COMMAND_INPUT_CAPACITY];
	
	struct aliasContainer_t *next;
	struct aliasContainer_t *prev;
}aliasContainer;

//List for History
struct cmdContainer_t {	
	int cmdNo;
	char cmdString[COMMAND_INPUT_CAPACITY];
	
	struct cmdContainer_t *next;
	struct cmdContainer_t *prev;
}cmdContainer;

//List for Environment
struct envPath_t{
	char path[COMMAND_INPUT_CAPACITY];
	struct envPath_t *next;
	struct envPath_t *prev;
}envPath;

//A buffer to hold verbose print
char printBuf[1024];

//All global scratchpad
pid_t scriptID;
int isScriptRunning;
int scriptPipe[2];
int startStopPipe[2];
int savedStdout;
int savedStdin;
int isVerboseOn;
int underVerbose;

// Sample for Verbose print
// sprintf(printBuf,"Trying at :%s\n",cmd_path);verPrint(printBuf);memset(printBuf,0,sizeof(printBuf));
