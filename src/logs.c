#include "shell.h"

/* --------------------------------------------------------------------- 
 * funct : Pause logging 
 * args  : None.
 * ret   : None.
 * 
 * Detail: Send a trap via pipe to scripting process and not 
 * 		   to log in file until told next.
 * 
 * --------------------------------------------------------------------*/
void pauseLogging()
{
	if(isScriptRunning==TRUE)
	{
		write(startStopPipe[1],"+",1);
	}
}

/* --------------------------------------------------------------------- 
 * funct : Resume logging 
 * args  : None.
 * ret   : None.
 * 
 * Detail: Send a trap via pipe to scripting process and to start
 * 		   log in file.
 * 
 * --------------------------------------------------------------------*/
void resumeLogging()
{
	if(isScriptRunning==TRUE)
	{
		write(startStopPipe[1],"-",1);
	}
}

/* --------------------------------------------------------------------- 
 * funct : Stop logging 
 * args  : None.
 * ret   : None.
 * 
 * Detail: Send a trap via pipe to scripting process to stop
 * 		   log in file. Scripting process save log file and exit.
 * 
 * --------------------------------------------------------------------*/
void stopLogging(char *cmd[])
{
	printf("stop script\n");

	//Revert back to original stdout
	dup2(savedStdout,1);
	write(startStopPipe[1],"AB",2);
}

/* --------------------------------------------------------------------- 
 * funct : Start logging 
 * args  : None.
 * ret   : None.
 * 
 * Detail: Start a process that read from pipe and put data in a file.
 * 		   Parent process change its stdout to pipe to child process.
 * 		   
 * --------------------------------------------------------------------*/
void startLogging(char *cmd[])
{
	//Create Stream pipe
	pipe(startStopPipe);
	//Create signalling pipe
	pipe(scriptPipe);
	
	//Start new process
	scriptID = fork();
	
	if(scriptID == 0)
	{

		FILE *scriptFD;
		
		//File name validation, jump to default
		if(cmd[1] == NULL || strlen(cmd[1]) <= 0)
		{
			printf("Default log file myshell.log\n");
			scriptFD = fopen(DEFAULT_SHELL_LOG,"a");
		}
		else
		{
			scriptFD = fopen(cmd[1],"a");
		}
		
		//Child will not write
		close(scriptPipe[1]);
		close(startStopPipe[1]);
		
		//Make read call to pipe non blocking
		int flags = fcntl(startStopPipe[0], F_GETFL, 0);
		fcntl(startStopPipe[0], F_SETFL, flags | O_NONBLOCK);
		
		int flags1 = fcntl(scriptPipe[0], F_GETFL, 0);
		fcntl(scriptPipe[0], F_SETFL, flags | O_NONBLOCK);
		
		char aData[256];
		char bData[256];
		
		if(scriptFD <= 1)
		{
			printf("Can not create or write to file %s, check permission\n",cmd[1]);
			exit(0);
		}
		
		int i;
		int stopTemp=0;
		
		while(1)
		{
				//Continuously loop over to read from pipe, write to file.
				memset(aData,0,sizeof(aData));
				memset(bData,0,sizeof(bData));
			
				i = read(scriptPipe[0],aData,1);
			
				if(aData[0] == 0x1B){stopTemp = 1;}
				if(stopTemp == 1 && aData[0] == 0x5B){}
				
				if(i>0)
				{
					//check for signalling
					if(read(startStopPipe[0],bData,1) > 0)
					{
						if(bData[0] == '+')
						{
							underVerbose = TRUE;
						}
						else if(bData[0] == '-')
						{
							underVerbose = FALSE;
						}
						else
						{
							printf("Script stop\n");
							fclose(scriptFD);
							exit(0);
						}
					}
					
					printf("%c",aData[0]);
					fflush(stdout);
					
					if(i>0 && (stopTemp ==0))
					{
						if(underVerbose==FALSE)
						{
							//write to log file
							fwrite(aData, sizeof(char), 1, scriptFD);
						}
					}
				}
				
				if(stopTemp == 1 && aData[0] == 0x46){stopTemp = 0;}
				
				//check for signalling
				if(read(startStopPipe[0],bData,1) > 0)
				{
					if(bData[0] == '+')
					{
						underVerbose = TRUE;
					}
					else if(bData[0] == '-')
					{
						underVerbose = FALSE;
					}
					else
					{
						printf("Script stop\n");
						fclose(scriptFD);
						exit(0);
					}
				}
		}
	}
	else
	{
		//Parent will not read
		close(scriptPipe[0]);
		close(startStopPipe[0]);
		
		savedStdout = dup(1);
		savedStdin = dup(0);
		
		dup2(scriptPipe[1],1);
		
		printf("Scripting started\n");		
	}	
}
