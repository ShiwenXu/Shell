/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std;
extern "C" char * read_line();

    int mygetc(FILE *f)
    {
        static char *p;
        char ch;
        if (!isatty(0))
        {
            return getc(f);
        }
        if ( p == NULL || *p == 0)
        {
            char *s = read_line();
            p = s;
        }
        ch = *p;
        p++;

        return ch;

}

#undef getc
#define getc(f) mygetc(f)

static void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

int yyparse(void);
void sourcecheck(FILE *f)
{
    YY_BUFFER_STATE file = (yy_create_buffer(f, YY_BUF_SIZE));
    yypush_buffer_state(file);
    yyparse();
    yypop_buffer_state();
}
%}

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">>&" {
  return GREATGREATAMPERSAND;
}

">&" {
  return GREATAMPERSAND;
}

">>" {
  return GREATGREAT;
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}
"2>" {
  return TWOGREAT;
}


\$\([^\n)]*\) {
    
    char *commandline = strdup(yytext+2);
    if (commandline[strlen(commandline)-1] == ')')
    {
        commandline[strlen(commandline)-1] = '\0';
    }
    int tempin = dup(0);
    int tempout = dup(1);

    int pipein[2];
    int pipeout[2];
    pipe(pipein);
    pipe(pipeout);
    
    write(pipein[1],commandline,strlen(commandline));
    write(pipein[1], "\n", 1);
    write(pipein[1], "exit",4);
    write(pipein[1], "\n", 1);
    close(pipein[1]);
    
    dup2(pipein[0],0);
    close(pipein[0]);
    dup2(pipeout[1],1);
    close(pipeout[1]);

    int pid = fork();
    if (pid == 0)
    {
        cout << "wtf" << endl;
        execvp("/proc/self/exe", NULL);
        _exit(1);
    }
    else if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
    dup2(tempin,0);
    dup2(tempout, 1);
    close(tempin);
    close(tempout);

    char c;
    char *buffer = (char*)malloc(4096);
    int i = 0; 
    while (read(pipeout[0], &c, 1))
    {
        if (c == '\n' && buffer[i-1] != ' ')
        {
            buffer[i++] = ' ';
        }
        else 
        {
            
            buffer[i++] = c;
        }
    }
    buffer[i-1] = '\0';
    for (i = strlen(buffer); i >= 0; i--)
    {
        unput(buffer[i]);
    }
    free(buffer);
}


["][^\n\"]*["] {
  /*replace quotes*/
  yylval.cpp_string = new std::string(yytext + 1);
  for (int i = 0; i < yyleng; i++)
  {
    if (yylval.cpp_string->c_str()[i] == '"')
    {
        yylval.cpp_string->replace(i, 1, "\0");
    }
  }
  return WORD;
}

[^ \t\n|><&]*\\[^ \t\n] {
    
    yylval.cpp_string = new std::string(yytext);
    return WORD;
}

[^ \t\n|<>][^ \t\n|<>]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}




