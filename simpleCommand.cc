/*#include <cstdio>
#include <cstdlib>*/
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "string.h"
#include <sys/wait.h>
#include <sstream>
#include "simpleCommand.hh"
using namespace std;

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  char *argument_copy = strdup(argument->c_str());
  if (strchr(argument_copy,'\\') != NULL)
  {
    int i = 0; 
    char *new_arguments = (char*)malloc(strlen(argument_copy) * sizeof(char));
    while (*argument_copy)
    {
      if (*argument_copy != '\\')
      {
        new_arguments[i] = *argument_copy;
        i++;
      }
      else {
        if (*(argument_copy+1) != '\\')
        {
          if(*(argument_copy +1) == '>' || *(argument_copy+1) == '<' || *(argument_copy+1) == '&')
          {
            argument_copy = argument_copy +1;
            new_arguments[i++] = *(argument_copy);
          }
          else{
            new_arguments[i++] = *(argument_copy);
          }
        }
        else 
        {
          argument_copy = argument_copy + 2;
          new_arguments[i++] = '\\';
        }
      }
      argument_copy++;
    }
    new_arguments[i] = '\0';
    argument_copy = new_arguments;
    std::string s(strdup(argument_copy));
    *argument = s;
   }
  free(argument_copy);
  
  string * exp = checkExpansion(argument);
  string * exp2 = checkTilde(argument);

  
    
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}

std::string *SimpleCommand::checkExpansion(std::string * argument)
{
    char * arg_dup = strdup(argument->c_str());

    //find the first occurrence of these two special symbols
    char * dollar_sign = strchr(arg_dup,'$');
    char * brace = strchr(arg_dup,'{');
    
    char * new_argument = (char*)malloc(sizeof(argument) *  50);
    char * temp = new_argument;

    if (dollar_sign != NULL && brace != NULL)
    {
        //arguments before the dollar sign
        while (*arg_dup != '$')
        {
            *temp = *arg_dup;
            temp++; 
            arg_dup++;
        }
        *temp  = '\0';       
        //aguments inside the dollar sign
        while (dollar_sign)
        {
            if (dollar_sign[1] == '{')
            {
                char * starting_point = dollar_sign + 2;
                char * expansion_part = (char*)malloc(sizeof(argument)* 50);
                char * expansion = expansion_part;
                while (*starting_point != '}')
                { 
                    *expansion = *starting_point;
                    expansion++;
                    starting_point++; 
                }
                *expansion = '\0';    
                char *after_expansion;
                //check if it didn't store this environemntal variable
                if (getenv(expansion_part) == NULL)
                {   
		                //check if it is a special environmental variable
		                if (!strcmp(expansion_part, "$"))
		                {
                            std::ostringstream str_pid;
                            str_pid << getpid();
                            const std::string my_pid(str_pid.str());
                            strcat(new_argument, my_pid.c_str());   
		                } 
		                /*else if (!strcmp(expansion_part, "?"))
		                { 
                            int status = 0;
                            waitpid(getppid(), &status, 0);
                            
                            if (WIFEXITED(status))
                            {
                                status = WEXITSTATUS(status);
                                setenv("?", std::to_string(status).c_str(),1);
                            }
		                }
		                else if (!strcmp(expansion_part, "!"))
		                {
                                std::ostringstream str_pid;
                                str_pid << getppid();
                                const std::string my_pid(str_pid.str());
                                strcat(new_argument, my_pid.c_str());
		                }*/
                }
                else
                {
                    if (!strcmp(expansion_part, "SHELL"))
                    {
                        char *path = (char *)malloc(500);
                        if (readlink("/proc/self/exe", path, 500) != -1)
                        {
                            char *real_path = realpath(path, NULL);
                            strcat(new_argument, real_path);
                        }
                    }
                    else
                    {
                        after_expansion = getenv(expansion_part);
                        strcat(new_argument, after_expansion);  
                    }  
                }

                while(*(arg_dup-1) != '}')
                {
                    arg_dup++;
                }

                char *buf = (char*)malloc(sizeof(argument) * 50);
                char *tbuf = buf;
                
                while(*arg_dup != '$' && *arg_dup)
                {
                    *tbuf = *arg_dup;
                    tbuf++; 
                    arg_dup++;
                }
                *tbuf = '\0';
                strcat(new_argument, buf);
            }
            dollar_sign++;
            dollar_sign = strchr(dollar_sign,'$');
        }
        std::string s(strdup(new_argument));
        *argument = s;
        free(new_argument);
        return argument;
    }
    return NULL;
}

std::string *SimpleCommand::checkTilde(std::string * argument)
{
    char * arg_dup = strdup(argument->c_str());
    if (arg_dup[0] == '~')
    {
        if (strlen(arg_dup) == 1)
        {
            arg_dup = strdup(getenv("HOME"));
            std::string s(strdup(arg_dup));
            *argument = s;
            free(arg_dup);
            return argument;
        }
        else
        {
            if (arg_dup[1] == '/')
            {
                char *dir = strdup(getenv("HOME"));
                arg_dup++;
                arg_dup = strcat(dir, arg_dup);
                std::string s(strdup(arg_dup));
                *argument = s;
                free(arg_dup);
                return argument;
            }
            
            char *arguments = (char*) malloc (strlen(arg_dup) + 20);
            char *names = (char *) malloc (50);
            char *user = names;
            char *temp = arg_dup;
    
            temp++;
            
            while (*temp != '/' && *temp)
            {
                *(names++) = *(temp++);
            }
            *names = '\0';
        
            if (*temp)
            {
                arguments = strcat(getpwnam(user)->pw_dir, temp);
                arg_dup = strdup(arguments);
                std::string s(strdup(arg_dup));
                *argument = s;
                free(arg_dup);
                return argument;
            }
            else
            {
                arg_dup = strdup(getpwnam(user)->pw_dir);
                std::string s(strdup(arg_dup));
                *argument = s;
                free(arg_dup);
                return argument;
            }
        }
    }
    return NULL;
}

/*int SimpleCommand::find_first_quote()
{

  int index1 = -1;
  int index2 = -1;

  std::vector<std::string *>::size_type i;
  std::vector<std::string *>::size_type j;
  for (i = 0; i < _arguments.size(); i++)
  {
      j = 0;
      while (j < _arguments[i]->size())
      {
        if (_arguments[i]->at(j) == 34)
        {
          //havent find any quote before
          if (index1 == -1)
          {
            index1 = i;
          }
          //already found the first index
          else{
            index2 = i;
          } 

        }
        j++;
      }
  }
  //merge the arguments together
  string new_merged_arguments = _arguments[index1][1] + _arguments[index2][_arguments[index2]->size()-1];
  insertArgument(new_merged_arguments);
  return index1;
}*/
