#include "shell.h"

/* --------------------------------------------------------------------- 
 * funct : Init RC file check
 * args  : RC file name, other scratchpad variable
 * ret   : None.
 * 
 * Detail: This funciton open RC file name given in argument.
 * 		   Go through it line by line. Execute commands.
 * 		   Starting character '#' is for line comment
 * 
 * --------------------------------------------------------------------*/
void checkRcFile(char *fileName, char *cmdOrig, char *readString, char *command, char *args, char *cmd[])
{
	//Scratchpad initialize
	FILE *rcFile;
	int read,l;
	char *line=malloc(COMMAND_INPUT_CAPACITY);
	
	rcFile = fopen(fileName,"r+");
	
	//If file not exist!
	if(rcFile <= 1)
	{
		printf("No RC file to read\n");
		return;
	}
	
	printf("Executing %s\n",fileName);
	
	//Go through line by line
	while ((read = getline(&line, &l, rcFile)) != -1) 
	{
        if(line[0] != '#' && strlen(line)>1)
        {
			printf("%s", line);
			sprintf(readString,"%s\n",line);
         
			//We have command line ready to be parsed
			intermediatePipingCheck(cmdOrig,readString,command, args, cmd);       
		}
    }

    fclose(rcFile);	
}

/* --------------------------------------------------------------------- 
 * funct : A core function for command interpreter
 * args  : User raw input string, Scratchpad variables
 * ret   : None
 * 
 * Detail: This function implement core functionality of shell.
 * 		   It go over word by word and execute command as required.
 * 		   Create child process to execute command. Wait for child 
 *         to finish or go ahead if non-waiting is request.
 * 
 * --------------------------------------------------------------------*/
int coreParser(char *cmdOrig, char *readString, char *command, char *args, char *cmd[])
{
	//Copy user input for backup
	memcpy(cmdOrig,readString,COMMAND_INPUT_CAPACITY);
	
	//Convert user string to separate command words.
	enum inputType commandParseState = parseInput(readString, &command, &args, cmd);
	
	//If user didn't write anything.
	if( commandParseState == NO_COMMAND)
	{
		//This is to recover newline issue while scripting is on. 
		//When user hit enter, it should not reflect in log.
		if(isScriptRunning == TRUE)
		{
			printf("%s\n",readString);
			//Go up by one line.
			printf("\033[F");
		}
	}
	
	//If we have some user input
	if(commandParseState > NO_COMMAND)
	{
		//Alias command handler
		if(strstr(cmd[0],"alias") != NULL)
		{
			pushAlias(cmdOrig);
			pushHistory(cmdOrig);
			return 0;
		}
		
		//Check If we have this command in alias lists.
		if(isCommandAliased(cmdOrig,readString,cmd)==TRUE)
		{
			memcpy(cmdOrig,readString,COMMAND_INPUT_CAPACITY);
			printf("%s\nAliased to %s\n",cmdOrig,readString);
			memset(cmd,0,sizeof(cmd));
			commandParseState = parseInput(readString, &command, &args, cmd);
		}
		
		//Chech for special command '!!' to role over last command
		if(strstr(cmd[0],"!!") != NULL && strlen(readString) == strlen("!!"))
		{
			struct cmdContainer_t *temp;
			temp = &cmdContainer;
			
			while(temp->next != NULL){temp = temp->next;}
			
			//If had no history at all.
			if(temp->next == NULL && temp->prev == NULL)
			{
				printf("No history available\n");
				return 0;
			}
			
			//Fetching last command
			memcpy(readString, temp->cmdString,COMMAND_INPUT_CAPACITY);
			memcpy(cmdOrig,readString,COMMAND_INPUT_CAPACITY);
			
			//Give it back to parser
			printf("%s\n",readString);
			commandParseState = parseInput(readString, &command, &args, cmd);
						
		}
		
		//Check for special command '!#' to role over particular history
		if(cmd[0][0] == '!' && cmd[0][1] != '!' && strlen(cmd[0]) > 1)
		{
			int reqNumber = atoi(&cmd[0][1]);
			struct cmdContainer_t *temp;
			int cmdFound=FALSE;
			
			//Number validation
			if(reqNumber <= 0)
			{
				printf("Invalid number of history command requested\n");
				return 0;
			}
						
			//Going through list
			temp = &cmdContainer;
			
			while(temp->next != NULL)
			{
				temp = temp->next;
				
				if(temp->cmdNo + 1 == reqNumber)
				{
					cmdFound = TRUE;
					memcpy(readString, temp->cmdString,COMMAND_INPUT_CAPACITY);
					memcpy(cmdOrig,readString,COMMAND_INPUT_CAPACITY);
			
					printf("%s\n",readString);		
					commandParseState = parseInput(readString, &command, &args, cmd);
					break;
				}
			}
			
			//If given number of history not present
			if(cmdFound == FALSE)
			{
				printf("Number for history command not valid\n");
				return 0;
			}
		}
		
		//Exit command handler
		if(strstr(cmd[0],"exit") != NULL && (strlen(cmd[0]) == strlen("exit")))
		{
			pushHistory(cmdOrig);
			exit(0);
		}
		
		//History command handler
		if(strstr(cmd[0],"history") != NULL && (strlen(cmd[0]) == strlen("history")))
		{
			commandHistory(cmd);
			pushHistory(cmdOrig);
			return 0;
		}
		
		//Check for script start stop request
		if(strstr(cmd[0],"script") != NULL && (strlen(cmd[0]) == strlen("script")))
		{
			if(isScriptRunning == FALSE)
			{
				startLogging(cmd);
				isScriptRunning = TRUE;
			}
			else
			{
				printf("scripting is already enabled, use endscript to stop.\n");
			}
			pushHistory(cmdOrig);
			return 0;
		}
		
		//Chech for script start stop request
		if(strstr(cmd[0],"endscript") != NULL && (strlen(cmd[0]) == strlen("endscript")))
		{
			printf("script end\n");
			if(isScriptRunning == TRUE)
			{
				stopLogging(cmd);
				isScriptRunning = FALSE;
			}
			else
			{
				printf("scripting is already off\n");
			}
			pushHistory(cmdOrig);
			return 0;
		}
		
		//Check for 'set' type of request
		if(strstr(cmd[0],"set") != NULL && (strlen(cmd[0]) == strlen("set")))
		{
			setHandler(cmdOrig,readString,cmd);
			return 0;
		}
		
		//Check for additional command 'showpath'.
		if(strstr(cmd[0],"showpath") != NULL && (strlen(cmd[0]) == strlen("showpath")))
		{
			printEnv();
			return 0;
		}
	}
	
	
	//-----------------------------------------------------------------//
	//---------Start Executing Command, Based on Args------------------//
	
	switch(commandParseState)
	{
		case INPUT_ERROR:
			printf("Input Error, Could not understand %s \n",readString);
			break;
		case NO_COMMAND:
			break;
		case ONLY_COMMAND:
			processCommand(cmdOrig, readString,cmd,TRUE);
			break;
		case ARGS_AND_COMMAND:
			processCommand(cmdOrig, readString,cmd,TRUE);
			break;
		case ARGS_AND_COMMAND_DONT_WAIT:
			processCommand(cmdOrig, readString,cmd,FALSE);
			break;
		default:
			printf("Parsing Error\n");
			break;	
	}		
	
	return 0;
	
}

/* --------------------------------------------------------------------- 
 * funct : Command string parser
 * args  : User input string, Scratchpad memory.
 * ret   : ENUM state based on argument if any?
 * 
 * Detail: This funciton does string parsing of user input.
 * 		   Create different states based on arguments.
 *  
 * --------------------------------------------------------------------*/
enum inputType parseInput(const char *inString, char **comm, char **args, char *cmd[])
{
	int i = 0;
	char *parser = inString;
	
	//Null input validation
	if(strlen(inString)==0)
		return NO_COMMAND;
		
	//Max input validation
	if(strlen(inString)>MAX_COMMAND_STRING)
	{
		printf("User can not enter command worth more than %d bytes long, please try again\n",MAX_COMMAND_STRING);
		return NO_COMMAND;
	}
	
	//Split each word in a line and store it in cmd.
	while(1)
	{
		cmd[i++] = parser;
		
		parser = strstr(parser," ");
		
		if(parser == NULL)
		{
			break;
		}
		else
		{
			*parser = '\0';
			parser++;			
		}
	}
	
	//If we had just one command
	if(i==1)
		return ONLY_COMMAND;
		
	//If we had command with arguments
	if(i>1)
	{
		if(cmd[i-1][0]=='&')
		{
			cmd[i-1] = NULL;
			return ARGS_AND_COMMAND_DONT_WAIT;
		}
		else
			return ARGS_AND_COMMAND;
	}
		
	return INPUT_ERROR;
}

/* --------------------------------------------------------------------- 
 * funct : Execute command
 * args  : Command container, scratchpad memory
 * ret   : None.
 * 
 * Detail: This function execute actual command binary by forking
 * 		   a child process. It wait for child to get done. Or else 
 * 		   dont wait if last argument is character '&'.
 * 
 * --------------------------------------------------------------------*/
void processCommand(char *cmdOrig, char *cmdString, char *cmd[], int isWait)
{
	//Save this command to history
	pushHistory(cmdOrig);
	
	//Create a child
	pid_t p = fork();
	
	//---- From now, we are two process---
	//Check if we are child ?
	if(p == 0)
	{
		//Try executing commmand in default path settings,
		//If command found, new binary will take over this process.
		execvp(cmd[0],cmd);
		
		//We are here because last call failed.
		struct envPath_t *envTemp;
		envTemp = &envPath;
		
		//Check over for binary in different PATH variables 
		do
		{
			char cmd_path[COMMAND_INPUT_CAPACITY] = {0};
			
			//Automate /bin or /bin/ input.
			if(envTemp->path[strlen(envTemp->path)-1] == '/')
			{
				sprintf(cmd_path,"%s%s",envTemp->path,cmd[0]);
			}
			else
			{
				sprintf(cmd_path,"%s/%s",envTemp->path,cmd[0]);
			}
		
			//Verbose search path
			sprintf(printBuf,"Trying at :%s\n",cmd_path);verPrint(printBuf);memset(printBuf,0,sizeof(printBuf));//VER
			
			//Try command at new path
			int i = execvp(cmd_path,cmd);
			
			envTemp = envTemp->next;
			
		}while(envTemp != NULL);
		
		//We are here such that, there is no command in path variabls also.
		printf("No such command: %s \n",cmd[0]);
		exit(0);
	}
	
	//What parent process should while child is on work ? 
	else
	{
		//Wait for command to get over
		if(isWait == TRUE)
		{
			int status;
			while(waitpid(p, &status, 0) == -1);
		}
		else
		{	
			return;
		}
	}
}
