#include <cstdio>
#include <stdlib.h>
#include "shell.hh"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>

using namespace std;
int yyparse(void);
void yyrestart(FILE * file);
int * list_of_pid;

void Shell::prompt() {
	if(isatty(0)){
	 printf("myshell>");
  	 fflush(stdout);    
	}
}

extern "C" void disp( int sig )
{
    printf("\n");
    Shell::prompt();
    
}

extern "C" void zombieElimination(int sig)
{
    int pid = wait3(0, 0, NULL);
    //value of pid = 1:  meaning wait for any child process.
    //option:WNOHANG return immediately if no child has exited.
    while(waitpid(-1, NULL, WNOHANG) > 0); 
    //waitpid returns process id on success;
    
    int flag = 0;
    
    int y;
    for (y = 0; y < 1000; y++)
    {
        if (list_of_pid[y] == pid)
            flag = 1;
    }
    
    if (flag == 1)
    {
        printf("[%d] exited.\n", pid);    
        Shell::prompt();
    }
}

int main() {    
 
    struct sigaction sa;
    sa.sa_handler = disp;
    sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGINT, &sa, NULL)){
	    perror("sigaction");
	    exit(2);
	}

	list_of_pid = (int*)(malloc(sizeof(int)*1000));
	struct sigaction sa2;
	sa2.sa_handler = zombieElimination;
	sigemptyset(&sa2.sa_mask);
	sa2.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD,&sa2, NULL))
	{
		perror("sigaction");
		exit(2);
	}
    FILE *fd = fopen(".shellrc", "r");
    if (fd)
    {
        yyrestart(fd);
        yyparse();
        yyrestart(stdin);
        fclose(fd);
    }
    else
    {
        Shell::prompt();
    }

    /*char *path = (char *)malloc(50);
    readlink("proc/self/exe", path, 50);
    printf("%s\n", path);*/
	yyparse();  
}



Command Shell::_currentCommand;
