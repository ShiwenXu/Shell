#ifndef shell_hh
#define shell_hh

#include "command.hh"
#include <signal.h>

extern int *list_of_pid;
struct Shell {


  static void prompt();

  static Command _currentCommand;

};

   void sourcecheck(FILE *f);



#endif
