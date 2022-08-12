
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
#include <unistd.h>

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
	return LESS;
}

">>" {
	return GREATGREAT;
}

"2>" {
	return TWOGREAT;
}

"|" {
	return PIPE;
}

"&" {
	return AMPERSAND;
}

">&" {
	return GREATAMPERSAND;
}

">>&" {
	return GREATGREATAMPERSAND;
}

\$\([^\n]*\) {
	std::string str = yytext;
	std::string sstr = str.substr(2);
	std::size_t	pos = str.find(")");
	std::string temp = sstr.substr(0,pos-2);
	char* subshell = const_cast<char*> (temp.c_str());

	int tempin = dup(0);
	int tempout = dup(1);

	int fdpipein[2];	
	pipe(fdpipein);

	int fdpipeout[2];	
	pipe(fdpipeout);

	write(fdpipein[1], subshell, strlen(subshell));
	write(fdpipein[1], "\n", 1);
	close(fdpipein[1]);	

	dup2(fdpipein[0], 0);	
	close(fdpipein[0]);	

	dup2(fdpipeout[1], 1);	
	close(fdpipeout[1]);

	int ret = fork();

	if (ret == 0) {
		execvp("/proc/self/exe", NULL);
		_exit(1);
	} else if (ret < 0) {
		perror("fork");
		exit(1);
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	close(tempout);

	char c;
	char * buffer = (char *) malloc(4096);
	int i = 0;
	
	while (read(fdpipeout[0], &c, 1)) {
		if (c == '\n') {
			buffer[i++] = ' ';
		}
		else {
			buffer [i++] = c;
		}
	}
	buffer[i] = '\0';

	for (int j = i - 1; j >= 0; j--) {	
		myunputc(buffer[j]);
	}
}

["][^\n\"]*["] {
	yylval.cpp_string = new std::string(yytext);
	*yylval.cpp_string = yylval.cpp_string->substr(1, yylval.cpp_string->length()-2);

	return WORD;
}

[^ \t\n|><&]*\\[^ \t\n]* {
  int i = 0;
	char * escChar;
	escChar = new char[strlen(yytext)*2];
	char * args = yytext;
	while ( *args ) {
		if ( *args == '\\' ) {	
			escChar[i] = *(++args);
			i++;	
		} else {
			escChar[i] = *args;
			i++;
		}
		args++;
	}
	escChar[i] = '\0';
	yylval.cpp_string = new std::string(escChar);
	return WORD;
	
}

[^ \t\n][^ \t\n]* {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;	
}
