// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed: Caleb Erhard Date: 02/18/2026

// 3460:426 Lab 1 - Basic C shell rev. 9/10/2020

/* Basic shell */

/*
 * This is a very minimal shell. It finds an executable in the
 * PATH, then loads it and executes it (using execv). Since
 * it uses "." (dot) as a separator, it cannot handle file
 * names like "minishell.h"
 *
 * The focus on this exercise is to use fork, PATH variables,
 * and execv. 
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>     // malloc
#include <sys/wait.h>   // wait / waitpid

#define MAX_ARGS		64
#define MAX_ARG_LEN		16
#define MAX_LINE_LEN	80
#define WHITESPACE		" ,\t\n"

struct command_t {
	// the name of the command to be executed
    char *name;
	// holds the number of command line arguments passed to the program
	int argc;
	// an array of strings that stores the command-line arguments passed to a program when it is executed
	char *argv[MAX_ARGS];
};

/* Function prototypes */
int parseCommand(char *, struct command_t *);
void printPrompt();
void readCommand(char *);

int main(int argc, char *argv[]) {
	int pid;
	int status;
	char cmdLine[MAX_LINE_LEN];
	struct command_t command;

	while (true) {
		printPrompt();
	    /* Read the command line and parse it */
	    readCommand(cmdLine);
	    parseCommand(cmdLine, &command);
	    command.argv[command.argc] = NULL;

		/* Check if the command is empty. occurrs when the user presses enter */
		if (command.argc == 0 || command.argv[0] == NULL || command.argv[0][0] == '\0') {
    		continue;
		}

		if (strcmp(command.argv[0], "C") == 0) {
			command.argv[0] = "cp";
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "D") == 0) {
			command.argv[0] = "rm";
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "P") == 0) {
			command.argv[0] = "more";
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "W") == 0) {
			command.argv[0] = "clear";
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "Q") == 0) {
    		break;
		}
		else if (strcmp(command.argv[0], "M") == 0) {
			// If no file name provided
			if (command.argc < 2) {
				printf("M: missing file name\n");
				continue;
			}

			// Replace "M" with "nano"
			command.argv[0] = "nano";
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "E") == 0) {
			bool printed = false;

			for (int i = 1; i < command.argc; i++) {

				if (command.argv[i] == NULL || command.argv[i][0] == '\0') {
					continue;
				}

				if (printed) {
					putchar(' ');
				}

				fputs(command.argv[i], stdout);
				printed = true;
			}
			if (printed) {
				putchar('\n');
			}

			continue;
		}
	  
		/* Create a child process to execute the command */
	    if ((pid = fork()) == 0) {
	       /* Child executing command */
	       execvp(command.name, command.argv);
		   perror("execvp");   // print an error message if execvp fails
    	   exit(1);
	    }
	    /* Wait for the child to terminate */
	    waitpid(pid, &status, 0);
	}

	/* Shell termination */
	printf("\n\n shell: Terminating successfully\n");
	return 0;
}

/* End basic shell */

/* Parse Command function */

/* Determine command name and construct the parameter list.
 * This function will build argv[] and set the argc value.
 * argc is the number of "tokens" or words on the command line
 * argv[] is an array of strings (pointers to char *). The last
 * element in argv[] must be NULL. As we scan the command line
 * from the left, the first token goes in argv[0], the second in
 * argv[1], and so on. Each time we add a token to argv[],
 * we increment argc.
 */
int parseCommand(char *cLine, struct command_t *cmd) {
    int argc;
	char **clPtr;
	/* Initialization */
	clPtr = &cLine;	/* cLine is the command line */
	argc = 0;
	cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN);
	/* Fill argv[] */
	while ((cmd->argv[argc] = strsep(clPtr, WHITESPACE)) != NULL) {
	    cmd->argv[++argc] = (char *) malloc(MAX_ARG_LEN);
	}

	/* Set the command name and argc */
	cmd->argc = argc-1;
	cmd->name = malloc(strlen(cmd->argv[0]) + 1);
	strcpy(cmd->name, cmd->argv[0]);
	return 1;
}

/* End parseCommand function */

/* Print prompt and read command functions - Nutt pp. 79-80 */

void printPrompt() {
    /* Build the prompt string to have the machine name,
	 * current directory, or other desired information
	 */
	printf("linux (ce90)|> ");
    // force the prompt to display immediately (don't wait for newline)
    fflush(stdout);
}

void readCommand(char *buffer) {
	/* This code uses any set of I/O functions, such as those in
	 * the stdio library to read the entire command line into
	 * the buffer. This implementation is greatly simplified,
	 * but it does the job.
	 */
    fgets(buffer, 80, stdin);
}

/* End printPrompt and readCommand */