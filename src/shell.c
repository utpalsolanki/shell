/* --------------------------------------------------------------------- 
 * CS543  	: Operating System
 * 			  Drexel University
 * Task   	: Homework 3
 * Student	: Utpal Solanki, Graduate Student @ ECE Department.
 * Detail 	: Designing a custom shell that allows traditional terminal
 * 			  like interaction. 
 * --------------------------------------------------------------------*/

#include "shell.h"

/* --------------------------------------------------------------------- 
 * main  : Code Entry Point, runs forever to listen commands.
 * args  : Accept NO arguments from user.
 * ret   : Never return, until exit call or terminated.
 * 
 * Detail: None.
 * 
 * --------------------------------------------------------------------*/
int main()
{
	//----------Initialization of Global Dataset-----------------------//
	int countIn=0;
	char getChar=0;
	char *readString;
	char *command;
	char *args;
	char *cmd[MAX_ARGS] = {NULL};
	char cmdOrig[COMMAND_INPUT_CAPACITY] = {NULL};
	
	isScriptRunning = FALSE;
	isVerboseOn = FALSE;
	underVerbose = FALSE;
	
	cmdContainer.cmdNo = 0;
	cmdContainer.cmdString[0] = NULL;
	cmdContainer.next = NULL;
	cmdContainer.prev = NULL;
	
	aliasContainer.aliasBase[0] = NULL;
	aliasContainer.cmdString[0] = NULL;
	aliasContainer.next = NULL;
	aliasContainer.prev = NULL;
	
	sprintf(envPath.path,".");
	envPath.next = NULL;
	envPath.prev = NULL;
	
	//Alloccating memory for user input space
	readString = malloc(COMMAND_INPUT_CAPACITY);
	
	if(readString == NULL)
	{
		perror("*** startup failed ***\n");
		exit(0);
	}
	
	//Check for RC file, initialize commands if available
	checkRcFile(DEFAULE_RC_FILE,cmdOrig,readString,command, args, cmd);
	
	while(1)
	{
		//Clear previous work
		memset(readString,0,COMMAND_INPUT_CAPACITY);
		memset(cmdOrig,0,COMMAND_INPUT_CAPACITY);
		memset(cmd,0,sizeof(cmd));
		
		//Printing default shell handler
		printf(DEFAULT_SHELL_STRING);
		
		countIn=0;
		do
		{
			getChar = getc(stdin);
			readString[countIn++]=getChar;
			
			if(countIn>=COMMAND_INPUT_CAPACITY)
			{
				printf("\nCommand input can not exceed %d bytes\n",COMMAND_INPUT_CAPACITY);
				countIn=0;
				memset(readString,0,COMMAND_INPUT_CAPACITY);
			}
			
		}while(getChar != '\n');
		readString[countIn-1]='\0';
		
		//Pass user input for further inspection
		intermediatePipingCheck(cmdOrig,readString,command, args, cmd);
		
	}
	free(readString);
	
	return 0;
}

/* --------------------------------------------------------------------- 
 * funct : Check for alias
 * args  : Accept raw string from user
 * ret   : TRUE if there is alias available, else FALSE
 * 
 * Detail: Chech alias command through linked list.
 * 		   If found, it will replace alias command with
 * 		   original command into raw string.
 * 
 * --------------------------------------------------------------------*/
int isCommandAliased(char *cmdOrig, char *readString, char *cmd[])
{
	//Temp struct, will loop over the list
	struct aliasContainer_t *temp;	
	temp = &aliasContainer;
	
	//Start to end, check for alias
	while(temp->next != NULL)
	{
		temp = temp->next;
		
		//If we found alias, replace it with origianl and return true
		if(strstr(cmd[0],temp->aliasBase) != NULL && (strlen(cmd[0]) == strlen(temp->aliasBase)) )
		{
			sprintf(printBuf,"We found alias\n");verPrint(printBuf);memset(printBuf,0,sizeof(printBuf));//VER
			memset(readString,0,COMMAND_INPUT_CAPACITY);
			sprintf(readString,"%s",temp->cmdString);
			
			return TRUE;
		}
	}
	
	return FALSE;
}

/* --------------------------------------------------------------------- 
 * funct : Save alias
 * args  : Command container
 * ret   : None.
 * 
 * Detail: This function accept array of commands, check if alias is
 * 		   already available in linked list. Update if available or 
 * 		   append new alias to linked list. 
 * --------------------------------------------------------------------*/
void pushAlias(char *cmdMain)
{
	//Initializing-Declaring scratchpad memory
	int isUpdated = FALSE;
	char *aliasTarget;
	char *aliasSave;
	struct aliasContainer_t *temp;
	char *cmdOrig = malloc(COMMAND_INPUT_CAPACITY);
	
	printf("Alisa set: %s\n",cmdMain);
	
	memcpy(cmdOrig,cmdMain,COMMAND_INPUT_CAPACITY);
	
	//String parsing through base and target
	cmdOrig = strstr(cmdOrig," ");
	VALIDATE_SUCCESS(cmdOrig,"Alias command usage error");
	cmdOrig++;
	
	aliasTarget = cmdOrig;
	cmdOrig = strstr(cmdOrig," ");
	VALIDATE_SUCCESS(cmdOrig,"Alias command usage error");
	cmdOrig[0] = '\0';
	cmdOrig++;
	
	cmdOrig = strstr(cmdOrig,"\"");
	VALIDATE_SUCCESS(cmdOrig,"Alias command usage error");
	cmdOrig++;
	
	aliasSave = cmdOrig;
	cmdOrig = strstr(cmdOrig,"\"");
	VALIDATE_SUCCESS(cmdOrig,"Alias command usage error");
	cmdOrig[0] = '\0';
	
	// Search through linked list, if alias is already present ?
	temp = &aliasContainer;
	while(temp->next != NULL)
	{
		temp = temp->next;
		//If alias is present, update it and return
		if(strstr(temp->aliasBase,aliasTarget) != NULL && (strlen(temp->aliasBase) == strlen(aliasTarget)))
		{
			memset(temp->aliasBase,0,COMMAND_INPUT_CAPACITY);
			memset(temp->cmdString,0,COMMAND_INPUT_CAPACITY);
			sprintf(temp->aliasBase,"%s",aliasTarget);
			sprintf(temp->cmdString,"%s",aliasSave);
			
			isUpdated = TRUE;
			return;
		}
	}
	
	struct aliasContainer_t *aliasTemp = malloc(sizeof(struct aliasContainer_t));

	//Storing new alias to temporary container
	sprintf(aliasTemp->aliasBase,"%s",aliasTarget);
	sprintf(aliasTemp->cmdString,"%s",aliasSave);
	
	//Appending new alias to linked list
	aliasTemp->prev = temp;
	temp->next = aliasTemp;

}
/* --------------------------------------------------------------------- 
 * funct : Save history
 * args  : Command container
 * ret   : None.
 * 
 * Detail: This function accept raw string and append it to command
 * 		   linked list history.
 * 
 * --------------------------------------------------------------------*/
void pushHistory(char *cmdOrig)
{
	//Scratchpad declaration
	int cmdNumber = 0;
	struct cmdContainer_t *cmdPtr;
	struct cmdContainer_t *cmdTemp;
	
	//Looping over to reach history list end
	cmdPtr = &cmdContainer;
	while(cmdPtr->next != NULL)
	{
		cmdPtr = cmdPtr->next;
		cmdNumber++;
	}
	cmdPtr->next = malloc(sizeof(struct cmdContainer_t));
	cmdTemp = cmdPtr;
	cmdPtr = cmdPtr->next;
	
	//Updating and appending recent command to list
	cmdPtr->cmdNo = cmdNumber;
	sprintf(cmdPtr->cmdString,"%s",cmdOrig);
	cmdPtr->next = NULL;
	cmdPtr->prev = cmdTemp;
}


/* --------------------------------------------------------------------- 
 * funct : History command handler
 * args  : Command container
 * ret   : None.
 * 
 * Detail: This function handles history command from user.
 * 		   Display linked list from most recent to least commands.
 * 
 * --------------------------------------------------------------------*/
void commandHistory(char *cmd[])
{
	int i = MAX_COMMAND_IN_HISTORY;
	struct cmdContainer_t *tempPtr;
	
	//Command validation
	if(cmd[1] != NULL)
	{
		printf("history command can not expect any argument\n");
		return;
	}
	
	//Reaching history list end
	tempPtr = &cmdContainer;	
	while(tempPtr->next != NULL)
	{
		tempPtr = tempPtr->next;
	}
	
	//Rolling back to print history
	while(tempPtr->prev != NULL)
	{
		printf("%d\t%s\n",(tempPtr->cmdNo)+1, tempPtr->cmdString);
		tempPtr = tempPtr->prev;
		
		if(i-- <= 1)
			break;
	}
	
	//Check if we had empty list
	if(i == MAX_COMMAND_IN_HISTORY)
		printf("No history available\n");
}
