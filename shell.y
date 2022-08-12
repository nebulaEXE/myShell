
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
%token NOTOKEN GREAT NEWLINE PIPE GREATGREAT AMPERSAND LESS GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
int yylex();

%}

%%

goal: command_list;

arg_list: 
  arg_list WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $2->c_str());
    Command::_currentSimpleCommand->insertArgument( $2 );\
  }
  | /*empty*/ 
  ;

cmd_and_args: 
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  arg_list {
    Shell::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

pipe_list: 
  pipe_list PIPE cmd_and_args 
  | cmd_and_args 
  ; 

io_modifier: 
  GREATGREAT WORD {
    if (Shell::_currentCommand._outFile != NULL ) {
		printf("Ambiguous output redirect.\n");
		exit(1);
	}
  Shell::_currentCommand._append = 1;
	Shell::_currentCommand._outFile = $2;
  }
  
  | GREAT WORD {
    if (Shell::_currentCommand._outFile != NULL ) {
		printf("Ambiguous output redirect.\n");
		exit(1);
	}
    Shell::_currentCommand._outFile = $2;
  }
  
  | GREATGREATAMPERSAND WORD {
    if (Shell::_currentCommand._outFile != NULL ) {
		printf("Ambiguous output redirect.\n");
		exit(1);
	}
  Shell::_currentCommand._append = 1;
	Shell::_currentCommand._outFile = $2;
	Shell::_currentCommand._errFile = $2;
  }
  
  | GREATAMPERSAND WORD {
    if (Shell::_currentCommand._outFile != NULL ){
		printf("Ambiguous output redirect.\n");
		exit(1);
	}
	Shell::_currentCommand._outFile = $2;
	Shell::_currentCommand._errFile = $2;
  }
  
  | LESS WORD {
    if (Shell::_currentCommand._inFile != NULL ) {
		printf("Ambiguous output redirect.\n");
		exit(1);
	}
	//printf("   Yacc: insert input \"%s\"\n", $2->c_str());
	Shell::_currentCommand._inFile = $2;
  }
  
  | TWOGREAT WORD {
    //printf("   Yacc: insert input \"%s\"\n", $2->c_str());
	  Shell::_currentCommand._errFile = $2;
  }
  ; 

io_modifier_list: 
  io_modifier_list io_modifier 
  | /*empty*/ 
  ; 
  
background_optional:  
  AMPERSAND {
    Shell::_currentCommand._background = true;
  }
  | /*empty*/ 
  ; 

command_line: 
  pipe_list io_modifier_list background_optional NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE /*accept empty cmd line*/ 
  | error NEWLINE{yyerrok;} 
   ;          /*error recovery*/ 

command_list :  
  command_line
  | command_list command_line 
    ;/* command loop*/ 

%%

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