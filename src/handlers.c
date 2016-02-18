#include "shell.h"

/* --------------------------------------------------------------------- 
 * funct : Verbose printing
 * args  : Data to print
 * ret   : None.
 * 
 * Detail: This function print verbose info if verbose mode is on.
 * 
 * --------------------------------------------------------------------*/
void verPrint( char *data )
{
	if(isVerboseOn==TRUE)
	{
		//Verbose print should not go to logfile
		pauseLogging();
		usleep(1000);

		printf( data );

		//Revert to previous settings
		usleep(1000);
		resumeLogging();		
	}
}

/* --------------------------------------------------------------------- 
 * funct : Set Environment Path
 * args  : Command container, Scratchpad memory
 * ret   : None.
 * 
 * Detail: This function set environment path in linked list.
 * 
 * --------------------------------------------------------------------*/
void setEnvPath(char *cmdOrig, char *readString, char *cmd[])
{
	
	//set path command validation
	if(strstr(cmdOrig,"(") == NULL || strstr(cmdOrig,")") == NULL)
	{
		printf("Invalid usage of set command\n");
		printf("Usage example: set path (. /bin /usr/bin)\n");
		return;
	}
	
	//Scratchpad memory init
	char *cmdFormat = malloc(strlen(cmdOrig));
	char *tracer;
	char *pathSetTemp;
	char isLastPath=FALSE;
	
	memcpy(cmdFormat,cmdOrig,strlen(cmdOrig));
	
	tracer = strstr(cmdFormat,"(");
	tracer++;
	
	//Loop over the path link list
	do
	{
		pathSetTemp = tracer;
		if(strstr(tracer," ")==NULL)
		{
			tracer = strstr(tracer,")");	
			isLastPath = TRUE;
		}
		else
		{
			tracer = strstr(tracer," ");
		}
		tracer[0] = '\0';		
		
		struct envPath_t *envTemp,*lastTemp;
		envTemp = &envPath;
		
		do
		{
			lastTemp = envTemp;
			envTemp = envTemp->next;
			
		}while(envTemp != NULL);
		
		envTemp = lastTemp;
		
		struct envPath_t *tempStruct = malloc(sizeof(struct envPath_t));	
		
		//Save new path variable
		sprintf(tempStruct->path,"%s",pathSetTemp);
		tempStruct->prev = envTemp;
		envTemp->next = tempStruct;
		
		tracer++;
		
	}while(isLastPath == FALSE);
	
}

/* --------------------------------------------------------------------- 
 * funct : Print current envrionment
 * args  : None.
 * ret   : None.
 * 
 * Detail: None.
 * 
 * --------------------------------------------------------------------*/
void printEnv()
{
	struct envPath_t *envTemp;
	envTemp = &envPath;
	
	do{
		printf(":%s ",envTemp->path);
		envTemp = envTemp->next;
	}while(envTemp != NULL);
	
	printf("\n");
	
}

/* --------------------------------------------------------------------- 
 * funct : Start verbose print
 * args  : Scratchpad memory
 * ret   : None.
 * 
 * Detail: start verbose printing.
 * 
 * --------------------------------------------------------------------*/
void setVerboseEnv(char *cmdOrig, char *readString, char *cmd[])
{
	
	//on off string validation
	if(strstr(cmd[2],"on") != NULL && (strlen(cmd[2]) == strlen("on")))
	{
		isVerboseOn=TRUE;
	}
	else if(strstr(cmd[2],"off") != NULL && (strlen(cmd[2]) == strlen("off")))
	{
		isVerboseOn=FALSE;
	}
	else
	{
		printf("Verbose command usage: set verbose on\n");
	}
}

/* --------------------------------------------------------------------- 
 * funct : Set Command handler
 * args  : Scratchpad memory
 * ret   : None
 * 
 * Detail: 'set' type command handler
 * 
 * --------------------------------------------------------------------*/
void setHandler(char *cmdOrig, char *readString, char *cmd[])
{
	//set command must have at least one argument
	if(cmd[1] == NULL || strlen(cmd[1])<1)
	{
		printf("Invalid usage of set command\n");
		return;
	}
	else if(strstr(cmd[1],"path") != NULL && (strlen(cmd[1]) == strlen("path")))
	{
		setEnvPath(cmdOrig, readString, cmd);
	}
	else if(strstr(cmd[1],"verbose") != NULL && (strlen(cmd[1]) == strlen("verbose")))
	{
		setVerboseEnv(cmdOrig, readString, cmd);
	}
	else
	{
		printf("Invalid usage of set command\n");
	}
}
