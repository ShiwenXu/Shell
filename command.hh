#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "y.tab.hh"

#include <cstdio>
#include <cstdlib>
// Command Data Structure
using namespace std;
struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _alreadyexists;
  bool _background;

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );
  //std::string checkExpansion ( SimpleCommand * simpleCommand);
  void clear();
  void print();
  void builtin();
  void execute();

  static SimpleCommand *_currentSimpleCommand;

  char *latest_cmd;
};

#endif
