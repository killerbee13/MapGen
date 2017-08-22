#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

//Command-line help
char HelpStr[] =
"USAGE: sequences [(options|scripted-command) [...]]                           \n"
"OPTIONS:                                                                      \n"
"        -q      Quiet mode; do not print prompts (overrides -v)               \n"
"        -v      Verbose mode; simulate user input for scripted commands       \n"
"        -c      Continue; switch to interactive mode after evaluating scripted\n"
"commands                                                                      \n"
"        -h      Help; print this message and exit                             \n"
"                See \"h\" command for help with command syntax                \n"
"Each argument may be either a command or a series of options. All             \n"
"options are guaranteed to be processed before scripted commands are           \n"
"interpreted, and all commands are run in order.                               \n";

//Interactive main help
char iHelpStr[] =
"Commands are as follows: (Abbreviate to capital letters)                      \n"
"Echo (string...), eXecute (command (args...)), Probe trie (command), Help     \n"
" (command), Quit, Map (filename, xmin, ymin, width)                           \n"
"Input is accepted as \"e test test2\", \"M out.png -100 -10 25\", or \"q\"    \n";

#endif //STRINGS_H_INCLUDED