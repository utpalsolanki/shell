#include "shell.h"

/* --------------------------------------------------------------------- 
 * funct : Intermediate parser, to check pipe command
 * args  : User raw input, scratchpad memory
 * ret   : None.
 * 
 * Detail: This function put a probe in between user input and
 * 		   core parser. If there is pipe '|' keyword, this function will 
 * 		   bridge output of one command to input of another command.
 * 		
 * 		   This function uses recursive behavior to support multiple '|'
 * 		   command. 
 * --------------------------------------------------------------------*/

void intermediatePipingCheck(char *cmdOrig, char *readString, char *command, char *args, char *cmd[])
{
	if(isScriptRunning==TRUE)
	{
		write(scriptPipe[1],readString,strlen(readString));
	}
	
	if(strstr(readString,"|") == NULL)
	{
		coreParser(cmdOrig,readString,command, args, cmd);
	}
	else
	{
		//we have pipe request from user
		//lets split string before and after pipe command
		char *stringBefore = malloc(COMMAND_INPUT_CAPACITY);
		char *stringAfter  = malloc(COMMAND_INPUT_CAPACITY);
		
		memcpy(stringBefore,readString,strlen(readString));
		
		char *temp = strstr(stringBefore,"|");
		temp[0] = '\0';
		temp--;
		temp[0] = '\0';
		temp++;
		temp++;temp++;
		
		memcpy(stringAfter,temp,strlen(temp));
		
		//Split Two Process Here
		int pipeInOut[2];
		pipe(pipeInOut);
		
		pid_t pipeFork;
		
		//Here we go !
		pipeFork = fork();
		
		if(pipeFork == 0)
		{
			//Welcome to child process
			
			//Change stdin
			close(pipeInOut[1]);
			dup2(pipeInOut[0],0);
			
			char *commandNew;
			char *argsNew;
			char *cmdNew[MAX_ARGS] = {NULL};
			char cmdOrigNew[COMMAND_INPUT_CAPACITY] = {NULL};
			
							
			intermediatePipingCheck(cmdOrigNew, stringAfter, commandNew, argsNew, cmdNew);
			
			exit(0);
		}
		else
		{
			//Welcome to parent process
			close(pipeInOut[0]);
			int revertStdout = dup(1);
			
			//Change stdout
			dup2(pipeInOut[1],1);
			
			char *commandNew;
			char *argsNew;
			char *cmdNew[MAX_ARGS] = {NULL};
			char cmdOrigNew[COMMAND_INPUT_CAPACITY] = {NULL};
							
			intermediatePipingCheck(cmdOrigNew, stringBefore, commandNew, argsNew, cmdNew);
			
			dup2(revertStdout,1);
		}
	}
}
