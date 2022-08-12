#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "shell.hh"

int yyparse(void);

void Shell::prompt() {
  if (isatty(0)) {
    printf("myshell>");
    fflush(stdout);
  }
  fflush(stdout);
}

extern "C" void ctrlc_sigHandler (int sig) {
	if (sig == SIGINT) {
    Shell::_currentCommand.clear();
  	printf("\n");
  	Shell::prompt();
	}
}

extern "C" void zombie_sigHandler (int sig) {
	pid_t pid = wait3(0, 0, NULL);
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		//printf("\n[%d] exited.", pid);
	}
	
}
	


int main() {
	
	struct sigaction sa; //cntrlc handler
	sa.sa_handler = ctrlc_sigHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &sa, NULL)) {
		perror("sigaction");
		exit(2);
	}

	struct sigaction sa2; //zombie handler
	sa2.sa_handler = zombie_sigHandler;
	sigemptyset(&sa2.sa_mask);
	sa2.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa2, NULL)) {
		perror("sigaction");
		exit(2);
	}

  if (isatty(0)) {
        Shell::prompt();
    }
  yyparse();
}

Command Shell::_currentCommand;
