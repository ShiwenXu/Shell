/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include "command.hh"
#include "shell.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <typeinfo>
#include <string>
using namespace std;

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        if (_outFile == _errFile)
        {
            delete _outFile;
	    _errFile = NULL;
        }
	else
        {
	    delete _outFile;
        }
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}


void Command::execute() {
    int ret;
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
//    print();

    /* Print contents of Command data structure
    print();

    Add execution here
    For every simple command fork a new process
    Setup i/o redirection
    and call exec*/
            //create a child process
        if (!strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"exit"))
        {
            //printf("Good bye!!\n");
            exit(0);
        }
        
        else if(!strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "setenv"))
        {
            int error = setenv(_simpleCommands[0]->_arguments[1]->c_str(), _simpleCommands[0]->_arguments[2]->c_str(), 1);
            if(error) {
                perror("setenv");
            }
            clear();
            Shell::prompt();
            return;
        }

        else if(!strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "unsetenv")){
            int error = unsetenv(_simpleCommands[0]->_arguments[1]->c_str());
            if(error) {
                perror("unsetenv");
            }
            clear();
            Shell::prompt();
            return;
        }

        else if(strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "cd") == 0){
            int error;
            if(_simpleCommands[0]->_arguments.size() == 1){ 
                error = chdir(getenv("HOME"));
            }
            else {
                char * temp = (char *)malloc(_simpleCommands[0]->_arguments[1]->size() * sizeof(char));
                if (_simpleCommands[0]->_arguments[1]->at(0) == '$')
                {
                    _simpleCommands[0]->_arguments[1]->replace(0,1,"\0");
                     if(_simpleCommands[0]->_arguments[1]->at(0) == '{')
                     { 
                        _simpleCommands[0]->_arguments[1]->replace(0,1,"\0");
                        for (int k = 0; k < _simpleCommands[0]->_arguments[1]->size(); k++)
                        {
                           /* if (_simpleCommands[i]->_arguments[1]->at(k) == '}')
                            {
                                _simpleCommands[i]->_arguments[1]->replace(k,1,"\0"); 
                            }*/
                            if (_simpleCommands[0]->_arguments[0]->at(k) != '}')
                            {
                                //printf("%c\n", _simpleCommands[i]->_arguments[1]->at(k));
                                *(temp+k) = _simpleCommands[0]->_arguments[1]->at(k);
                            }
                        }
                        error = chdir(getenv(temp));
                        //printf("%d\n", error);
                    }
                    else{
                        error = chdir(getenv(_simpleCommands[0]->_arguments[1]->c_str()));
                    }
                }
                else {
                    error = chdir(_simpleCommands[0]->_arguments[1]->c_str());
                }
            }
            if(error < 0){
                fprintf(stderr, "cd: can't cd to %s\n:", _simpleCommands[0]->_arguments[1]->c_str());
                
            }
             
            clear();
            Shell::prompt();
            return;
        }

    //dup default in/out/err
    int tmpin=dup(0);
    int tmpout=dup(1);
    int tmperr=dup(2);

    //set the initial input/output/errfile
    int fdin;
    int fdout;
    int fderr;

    //if there is a Input file
    if (_inFile) 
    {
	   //open the input file, read only
   	    fdin = open(_inFile->c_str(),O_RDONLY);
        if (fdin < 0)
        {
            exit(1);
        }
    }
    else 
    {
	//else, open the default input file
 	    fdin = dup(tmpin);
    } 
    if (_errFile)
    { 
        if (_alreadyexists == true)
        {
                fderr=open(_errFile->c_str(), O_CREAT|O_WRONLY|O_APPEND,0664);
        }
        else
        {
                fderr=open(_errFile->c_str(),O_CREAT|O_WRONLY|O_TRUNC,0664);

        }
	
        if (fderr <0)
        {
            exit(1);
        }
    }       //open the default err file;
    else
    {
        fderr = dup(tmperr);
        close(tmperr);
        close(tmpin);
    }

    for (std::vector<SimpleCommand *>::size_type i = 0; i < _simpleCommands.size(); i++)
    {
        //redirect the input file
        dup2(fdin, 0);
        close(fdin);
        dup2(fderr, 2);
        close(fderr);
         
        if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source"))
        {
            FILE * fd = fopen(_simpleCommands[0]->_arguments[1]->c_str(), "r");
            _simpleCommands.erase(_simpleCommands.begin() + i); 
            sourcecheck(fd);
        } 
        //if is at the end of command
        if (i == _simpleCommands.size()-1)
        {
            if (_outFile)
            {
                //output  already exists
                if (_alreadyexists == true)
                {
                    fdout=open(_outFile->c_str(),O_CREAT|O_WRONLY|O_APPEND,0664);
                    if (fdout < 0)
                    {
                        exit(1);
                    }
                }
                //output does not exits
                else
                {   
                    fdout=open(_outFile->c_str(),O_CREAT|O_WRONLY|O_TRUNC,0664);
                    if (fdout < 0)
                    {
                        exit(1);
                    }
                }
            }
            else if (_errFile)
            {
                //if append
                if (_alreadyexists == true)
                {
                    fderr=open(_errFile->c_str(), O_CREAT|O_WRONLY|O_APPEND,0664);
                    if (fderr <0)
                    {
                        exit(1);
                    }
                }
                else
                {
                    fderr=open(_errFile->c_str(),O_CREAT|O_WRONLY|O_TRUNC,0664);
                    if (fderr <0)
                    {
                        exit(1);
                    }
                }
            }        
            else
            {
                //if there is no ouput file, set it to default
                fdout = dup(tmpout);
                close(tmpout);
            }
        }
        else
        {   
            int fdpipe[2];
            pipe(fdpipe);
            //what is written in fdpipe[1] could be read from fdpipe[0];
            fdout=fdpipe[1];
            fdin=fdpipe[0];
        }
        //redirect output file
        dup2(fdout,1);
        close(fdout);
        close(fderr);

        ret =fork();
        if (ret == 0)
        { 
            if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv"))
            {
                char** p = environ;
                while (*p) 
                {
                    printf("%s\n", *p);
                    p++;
                }  
                exit(0);
            }
			else{
	            //child process
	            char** new_simplecommands = new char*[_simpleCommands[i]->_arguments.size()];
	            std::vector<std::string *>::size_type j;
	            for (j = 0; j < _simpleCommands[i]->_arguments.size(); ++j)
	            {
	                new_simplecommands[j] = const_cast<char*>(_simpleCommands[i]->_arguments[j]->c_str());
	            }
	            new_simplecommands[j] = NULL; 
                execvp(_simpleCommands[i]->_arguments[0]->c_str(), new_simplecommands);
	            perror("execvp");
	            _exit(1);
	        } 
      }
      else if (ret < 0)
      {
	       perror("fork");
           return;
	  }
    }
  
    dup2(tmpin,0);
    dup2(tmpout,1);
    dup2(tmperr, 2);
    
    if (_simpleCommands.size() > 0)
    {
        int size = _simpleCommands[_simpleCommands.size() -1]->_arguments.size();
        latest_cmd = strdup(_simpleCommands[_simpleCommands.size() - 1]->_arguments[size-1]->c_str());
    }
    close(tmpin);
    close(tmpout);
    close(tmperr);
    
     int status = 0; 
    if (!_background)
    { 
        waitpid(ret, &status, 0);
        
        if (WIFEXITED(status))
        {
               status =  WEXITSTATUS(status);
        }
        setenv("?", std::to_string(status).c_str(), 1); 
    }
    else
    {
        setenv("!", std::to_string(ret).c_str(), 1);
        /*int i;
        for (i = 0; i < 1000; i++)
        {
            //have not yet changed state, then 0 is returned;
            if (list_of_pid[i] == 0)
            {
                break;
            }
        }
        list_of_pid[i] = ret;*/

    }
    setenv("_", latest_cmd, 1);
    clear();
    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
