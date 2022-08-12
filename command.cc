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
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> 
#include <fstream>

#include "command.hh"
#include "shell.hh"


Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
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

    if (_outFile == _errFile) {
        if (_outFile) {
            _errFile = new std::string(*_outFile);
        }
    }

    if ( _outFile ) {
        delete _outFile;
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

    _append = false;
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
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }

    // Print contents of Command data structure
    //print();

    //Exit Handler
    if (!strcmp(_simpleCommands[0]->_arguments[0]->c_str(),"exit")) {
		printf( "Good Bye!!\n" ); 
		_exit(1);
	}

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    int tempin = dup(0);
    int tempout = dup(1);
    int temperr = dup(2);

    int fdin;
	int fdout;
	int fderr;
    
    int numSimpleCommands = _simpleCommands.size();
    int ret;

    if (_errFile) { //If >&, >>&, or 2>
        if (_append) {
            fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        } else {
            fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
        }
    } else {
        fderr = dup(temperr);
    }

    if (_inFile) { //If Input File Provided
        fdin = open(_inFile->c_str(), O_RDONLY);
        
    } else { //Use stdin
        fdin = dup(tempin);
    }

    for (int i = 0; i < numSimpleCommands; i++) {
        dup2(fdin, 0);
        close(fdin);

        if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv")){
			if(setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1)) {
				perror("setenv");
			}
			clear();
			Shell::prompt();
			return;
		} 
		if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv")){

			if(unsetenv(_simpleCommands[i]->_arguments[1]->c_str())) {
				perror("unsetenv");
			}
			clear();
			Shell::prompt();
			return;
		}
		if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd")) {
			if (_simpleCommands[i]->_arguments.size() == 1) {
				if (chdir(getenv("HOME"))) {
                    perror("cd");
                }
			} else {
				const char * path = _simpleCommands[i]->_arguments[1]->c_str();
				char message[1024] = "cd: can't cd to ";
				strcat(message, path);	
				if (chdir(path)) { 
                    perror(message);
                }
			}
			clear();
			Shell::prompt();
			return;
		}
		if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source")) {
        }

        if (i == numSimpleCommands - 1) {
            if (_outFile) {
                if (_append) {
					fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
				} else {
					fdout=open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
				}
            } else {
                fdout = dup(tempout);
            }
        } else { //Not Last Command
            int fdpipe[2];
            pipe(fdpipe);
            fdout = fdpipe[1];
            fdin = fdpipe[0];
        }

        dup2(fdout, 1); //Redirect Output
        close(fdout);
        
        ret = fork(); //Create Child Process(es)

        int simpleArgsNum = _simpleCommands[i]->_arguments.size();
		char ** args = new char*[simpleArgsNum + 1];
		for (int j = 0; j < simpleArgsNum; j++) {
			args[j] = strdup(_simpleCommands[i]->_arguments[j]->c_str());
		}
		args[simpleArgsNum] = NULL;
        if (ret == 0) { //Child Process
            if (!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
				char ** p = environ;

				while(*p != NULL){
					printf("%s\n", *p);
					p++;
				}
				fflush(stdout);
				_exit(1);
			}
            execvp(_simpleCommands[i]->_arguments[0]->c_str(), args);
			perror("execvp");
			_exit(1);
        } else if (ret < 0) {
            perror("fork");
            return;
        }
    }

    dup2(tempin,0);
    dup2(tempout,1);
    dup2(temperr,2);
    close(tempin);
    close(tempout);
    close(temperr);

    if (!_background) {
       waitpid(ret, NULL, 0);
    }
    
    // Clear to prepare for next command
    clear();

    // Print new prompt
    if (isatty(0)) {
        Shell::prompt();
    }
}

SimpleCommand * Command::_currentSimpleCommand;
