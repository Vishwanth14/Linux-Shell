#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>

  

void sighandler(int sig_num)//for trapping the ctrl+Z signal by resetting it every time
{
	signal(SIGTSTP, sighandler);
}
void sighandle(int sig_num)//for trapping the ctrl+C signal by resetting it every time
{
	signal(SIGINT, sighandle);
}

char** parseInput(char *buffer,char** arr)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	int i=0;
	char *found;
	while((found=strsep(&buffer," "))!=NULL)
	{
		if(strlen(found)==0)
		{
			continue;
		}
		arr[i]=found;
		i++;
	}
	arr[i]=NULL;

	return arr;
}


void executeCommand(char *buffer,int c){// execute a command
	char* top;//returns a pointer to the null terminated byte string
	char *command[2];
    int k=0;
		while((top=strsep(&buffer," "))!=NULL){
		command[k]=top;
		k++;
	}
	command[k]=NULL;

	int status=0;

	int rc=fork();//forking
	if(rc<0){
		printf("Error creating child process\n");
		exit(0);
	}
	else if(rc==0){
			int u=execvp(command[0],command);//executing the command
			if(u==-1){
				if(strcmp(command[0],"cd")!=0){
					printf("Shell: Incorrect command\n");
				}
			}
		
		exit(0);
	}
	else{
		  if(strcmp(command[0],"cd")==0){//if the given cmd is "cd" the parent process will move into the specified directory rather than the child process
			if(chdir(command[1])==-1){
				
				perror("Error changing to another directory.Returning to home\n");
				
			}
		}
		if(c!=0)//condition to check whether commands run in parallel or not.
		{                            
		  wait(NULL);             //check==0 for executing commands in parallel
	    }
	    else{
	    	while((rc = wait(&status))>0);
		}
		
    }
}
void executeParallelCommands(char *buffer,char **arr)
{
	// This function will run multiple commands in parallel
	int i,j,k; 
	char *found;
	i=0;
        while((found=strsep(&buffer,"&&"))!=NULL)
	{
		if(strlen(found)==0)
		{
			continue;
		}
		arr[i]=found;
		i++;
	}
	arr[i]=NULL;
	
	int size=i;
	
	j=0;
	for(i=0;i<size;i++)
	{
		
		executeCommand(arr[i],0);
		
	}
	
}
void executeSequentialCommands(char *buffer,char **arr)
{
	// This function will run multiple commands in parallel
	int i,j,k; 
	char *found;
	i=0;
        while((found=strsep(&buffer,"##"))!=NULL)
	{
		if(strlen(found)==0)
		{
			continue;
		}
		arr[i]=found;
		i++;
	}
	arr[i]=NULL;
	int size=i;
	
	j=0;
	for(i=0;i<size;i++)
	{
		
		executeCommand(arr[i],1);
		
	}
}	
void executeCommandRedirection(char *buffer,char **arr)
{
	// This function will run a single command with output redirected to an output file specificed by user
	int i; 
	char *found,*cmd[2],*cmd2[2];
	i=0;
        while((found=strsep(&buffer,">"))!=NULL)
	{
		if(strlen(found)==0)
		{
			continue;
		}
		arr[i]=found;
		i++;
	}
	arr[i]=NULL;
	char **ret,**ret1;
	ret=parseInput(arr[0],cmd);
	ret1=parseInput(arr[1],cmd2);
	int rc=fork();
	if(rc==0)
	{
		close(STDOUT_FILENO);
		open(ret1[0],O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		if(execvp(ret[0],ret)<0)
		{
			printf("Shell: Incorrect command\n");
			exit(1);
		}
	}
	else if(rc>0)
	{
		wait(NULL);
	}
}




int main()
{
   signal(SIGTSTP, sighandler);//signal handling.
   signal(SIGINT, sighandle);
   
while(1) //infinite loop,runs till exit command is used.
{
	//setting global var to their default value for every iteration.
	        //Value is used for every iteration,so they are renewed
	
	char currentWorkingDirectory[FILENAME_MAX];//char array for directory folder
	
	char *line,* buffer;//variables to store the input for executing
	
	size_t buffsize=64;//size of input
	
	line=(char *)malloc(buffsize * sizeof(char));
	  if( line == NULL){
           perror("Unable to allocate memory");
            exit(1);
    }     //create space for storing input
	
	getcwd(currentWorkingDirectory,FILENAME_MAX);//get current directory name
	
	printf("%s$",currentWorkingDirectory);//print current dir in the same fashion how command prompt does.
	
	getline(&line,&buffsize,stdin);//taking input
	
	if(line[0]==10)   // For Empty command,to not show any error msg
	{                 //10 is ASCII code for enter.
		continue;
	}
	buffer=(char*)malloc(buffsize * sizeof(char));
	  if( buffer == NULL){
           perror("Unable to allocate buffer");
            exit(1);
    }   
	for(int i=0;i<strlen(line)-1;i++)
	{
		buffer[i]=line[i];
	  }  
	
	int c=-1;
	//checking for symbols in the input
	if(strstr(buffer,"&&")!=NULL)
		{
			c=0;
		}
		else if(strstr(buffer,"##")!=NULL)
		{
			c=1;
		}
		else if(strstr(buffer,">")!=NULL)
		{
			c=2;
		}

	
	

	int k=strcmp(buffer,"exit");//if "exit" is typed the program exits.
	if(k==0){
		printf("Exiting shell...\n");
		exit(0);
	}
	char* arr[64];
	
	if(c==0){ //condition check to implement which type of function.
		executeParallelCommands(buffer,arr);
	}
	else if(c==1){
		executeSequentialCommands(buffer,arr);
	}
	else if(c==2){
		executeCommandRedirection(buffer,arr);
	}
	else 
		executeCommand(buffer,c);
		
	free(line);
	free(buffer);
	
}
   return 0;
} 
