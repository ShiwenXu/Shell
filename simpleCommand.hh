#ifndef simplcommand_hh
#define simplecommand_hh

#include <string>
#include <vector>

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;
  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  std::string * checkExpansion(std::string *argument);
  std::string * checkTilde (std::string *argument);
  void print();
};

#endif
