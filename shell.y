
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT LESS NEWLINE GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT PIPE AMPERSAND 

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"
#include "command.hh"
#include <sys/types.h>
#include <regex.h>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
using namespace std;
void yyerror(const char * s);
int yylex();

void expandWildCards(char *prefix, char *arg);
int cmpfunc(const void *file1, const void *file2);
void expandWildCardsIfNecessary(char *argument);
%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
  ;

simple_command:	
    piple_list iomodifier_list background_option NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE {Shell::prompt();}
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
      if ((strchr($1->c_str(),'?') && !strchr($1->c_str(), '{')) || strchr($1->c_str(),'*'))
      {
        char *add = (char*)malloc(sizeof(char) * 100);
        strcpy(add, $1->c_str());
        expandWildCardsIfNecessary(add);
      }
      else
      {
        Command::_currentSimpleCommand->insertArgument( $1 );
      }
  } 
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

piple_list:
    piple_list PIPE command_and_args
    |command_and_args
    ;

iomodifier_opt:
  GREAT WORD {
    if (!Shell::_currentCommand._outFile) 
     {
	Shell::_currentCommand._outFile = $2;
     }
     else{
	printf("Ambiguous output redirect.\n");
	exit(1);
     }
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());

  }
  |
  LESS WORD {
     //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
     Shell::_currentCommand._inFile = $2;
  }
  |
  GREATGREAT WORD {
     //printf("   Yacc: insert output at the end \"%s\"\n",$2->c_str());
     Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand._alreadyexists = true;
  }
  |
  GREATAMPERSAND WORD {
     //printf("   Yacc: insert both output and errfile \"%s\"\n", $2->c_str());
     Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand._errFile = $2;
  }
  | 
  GREATGREATAMPERSAND WORD {
     //printf("   Yacc: append both output and errfile \"%s\"\n", $2->c_str());
     Shell::_currentCommand._outFile = $2;
     Shell::_currentCommand._errFile = $2;
     Shell::_currentCommand._alreadyexists = true;
  }
  |
  TWOGREAT WORD {
     //printf("   Yacc: insert errfile \"%s\"\n", $2->c_str());
     Shell::_currentCommand._errFile = $2;
  }
  ;


iomodifier_list:
  iomodifier_list iomodifier_opt 
  | iomodifier_opt
  |/*empty*/
  ;

background_option:
   AMPERSAND{
      Shell::_currentCommand._background = true;
   }
   |/*empty*/
   ;


%%

//std::vector<string *> entries;
int maxEntries;
int nEntries;
char **entries;
bool exist = false;

void expandWildCardsIfNecessary(char * arg)
{
    maxEntries  = 20;
    nEntries = 0;
    entries = (char**)malloc(maxEntries * sizeof(char*)); 

    if (strchr(arg, '*') || strchr(arg, '?'))
    {
        expandWildCards(NULL, arg);
        if (exist == false)
        {
           string *s = new std::string(arg);
            Command::_currentSimpleCommand->insertArgument(s);

        }
        else{
        qsort(entries, nEntries, sizeof(char *), cmpfunc);
        //sort(entries.begin(), entries.end(), cmpfunc);
        for (int i = 0; i < nEntries; i++) 
        {
            string *entry = new std::string(entries[i]);
            Command::_currentSimpleCommand->insertArgument(entry);
        }
        }        
    }
    return;

}

int cmpfunc (const void *file1, const void *file2)
{
    const char *_file1 = *(const char **)file1;
    const char *_file2 = *(const char **)file2;

    return strcmp(_file1, _file2);
}

void expandWildCards(char *prefix, char * arg)
{   
    
    char *temp_arg = arg;
    char *save_copy = (char*)malloc(strlen(arg) + 10);
    char * direc = save_copy;
 
    if (*temp_arg == '/') 
    {
        *(save_copy++) = *(temp_arg++);
    }   
    while (*temp_arg != '/' && *temp_arg)
    {
        *(save_copy++) = *(temp_arg++);
    }
    *save_copy = '\0';

    if (strchr(direc, '*' ) || strchr(direc, '?'))
    {
        if (!prefix && arg[0] == '/')
        {
            prefix = strdup("/");
            direc++;
        }    
        char * reg = (char*)malloc(2*strlen(arg) + 10);
        char *a = direc;
        char *r = reg;
        
        *(r++) = '^';
        
        while (*a){
            if (*a == '/')
            {
                a++;
            } 
            if (*a == '*'){*(r++) = '.'; *(r++) = '*';}   
            else if (*a == '?'){ *(r++) = '.';}
            else if (*a == '.'){ *(r++) = '\\'; *(r++) = '.';}
            else { *(r++) = *a; }
            a++;
        }
        *(r++) = '$';
        *r = '\0'; 
        regex_t re;
        int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
        char *toOpen = strdup((prefix)?prefix:".");
        DIR *dir = opendir(toOpen); 
        if (dir == NULL)
        {
            perror("opendir");
            return;
        }    
        struct dirent *ent;
        regmatch_t match;   
        
        while ((ent = readdir(dir)) != NULL)
        {
            if (!regexec(&re, ent->d_name, 1, &match, 0))
            {
                if (*temp_arg)
                {
                    if (ent->d_type == DT_DIR)
                    {
                        char *nPrefix = (char*) malloc(150);
                        if (!strcmp(toOpen, ".")) nPrefix = strdup(ent->d_name);
                        else if (!strcmp(toOpen,"/")) sprintf(nPrefix, "%s%s", toOpen, ent->d_name);
                        else sprintf(nPrefix, "%s/%s", toOpen, ent->d_name);
                        expandWildCards(nPrefix, (*temp_arg == '/')?++temp_arg:temp_arg);
                    }
                }
                else
                {
                    if (nEntries == maxEntries)
                    {
                        maxEntries *= 2;
                        entries = (char**) realloc (entries, maxEntries * sizeof(char*));
                    }

                    char *argument = (char *) malloc(100);  
                    argument[0] = '\0';
                    if (prefix) sprintf(argument, "%s/%s", prefix, ent->d_name);
                    

                    if (ent->d_name[0] == '.')
                    {
                        if (*arg == '.')
                        {
                        	/*if (argument[0] != '\0')
                        	{
                            string s(strdup(argument));
                            string *args;
                            *args = s;
                        		entries.push_back(args);
                        	} 
                        	else{
                            string s(strdup(ent->d_name));
                            string *args;
                            *args = s;
                        		entries.push_back(args);
                        	}*/
                          exist = true;
                          entries[nEntries++] = (argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
                        }
                    }
                    else
                    {
                    	/*if(argument[0] != '\0')
                    	{
                          string s(strdup(argument));
                          string *args;
                          *args = s;
                          entries.push_back(args);
                    	} 
                    	else
                    	{
                          string s(strdup(ent->d_name));
                          string *args;
                          *args = s;
                        	entries.push_back(args);
                    	}*/
                        if (argument[0] != '\0')
                        {
                            exist = true;
                            entries[nEntries] = strdup(argument);
                            nEntries++;
                        }
                        else
                        {
                            exist = true;
                            entries[nEntries] = strdup(ent->d_name);
                            nEntries++;
                        }
                   } 
                }
            }
        }
        closedir(dir);
    }
    else
    {
      char *preTosend = (char *)malloc(100);
      if (prefix) sprintf(preTosend, "%s%s", prefix, direc);
      else preTosend = strdup(direc);

      if (*temp_arg) expandWildCards(preTosend, temp_arg);
    }
}



void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
