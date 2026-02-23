/*
 * ACADEMIC INTEGRITY PLEDGE
 *
 * - I have not used source code obtained from another student nor
 *   any other unauthorized source, either modified or unmodified.
 *
 * - All source code and documentation used in my program is either
 *   my original work or was derived by me from the source code
 *   published in the textbook for this course or presented in
 *   class.
 *
 * - I have not discussed coding details about this project with
 *   anyone other than my instructor. I understand that I may discuss
 *   the concepts of this program with other students and that another
 *   student may help me debug my program so long as neither of us
 *   writes anything during the discussion or modifies any computer
 *   file during the discussion.
 *
 * - I have violated neither the spirit nor letter of these restrictions.
 *
 * Signed: Caleb Erhard Date: 02/18/2026
 */

/* 3460:426 Lab 1 - Basic C shell rev. 9/10/2020 */

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
#include <stdlib.h>     /* malloc */
#include <sys/wait.h>   /* wait / waitpid */

#define MAX_ARGS		64
#define MAX_ARG_LEN		16
#define MAX_LINE_LEN	80
#define WHITESPACE		" ,\t\n"

struct command_t {
	/* the name of the command to be executed */
    char *name;
	/* holds the number of command line arguments passed to the program */
	int argc;
	/* an array of strings that stores the command-line arguments passed to a program when it is executed */
	char *argv[MAX_ARGS];
};

/* Function prototypes */
int parseCommand(char *, struct command_t *);
void printPrompt();
void readCommand(char *);
void printHelp();

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

		/* Map shorthand C command to cp */
		if (strcmp(command.argv[0], "C") == 0) {
			command.argv[0] = "cp";
			command.name = command.argv[0];
		}
		/* Map shorthand D command to rm */
		else if (strcmp(command.argv[0], "D") == 0) {
			command.argv[0] = "rm";
			command.name = command.argv[0];
		}
		/* Map shorthand P command to more */
		else if (strcmp(command.argv[0], "P") == 0) {
			command.argv[0] = "more";
			command.name = command.argv[0];
		}
		/* Map shorthand W command to clear */
		else if (strcmp(command.argv[0], "W") == 0) {
			command.argv[0] = "clear";
			command.name = command.argv[0];
		}
		/* Q exits the shell loop */
		else if (strcmp(command.argv[0], "Q") == 0) {
    		break;
		}
		else if (strcmp(command.argv[0], "M") == 0) {
			/* M requires a file name argument */
			if (command.argc < 2) {
				printf("M: missing file name\n");
				continue;
			}

			/* Replace M with nano so the file opens in the editor */
			command.argv[0] = "nano";
			command.name = command.argv[0];
		}
		/* E behaves like echo by printing arguments after the command token */
		else if (strcmp(command.argv[0], "E") == 0) {
			bool printed = false;

			for (int i = 1; i < command.argc; i++) {
				/* Skip empty tokens produced by whitespace parsing */
				if (command.argv[i] == NULL || command.argv[i][0] == '\0') {
					continue;
				}

				/* Add spacing only between printed arguments */
				if (printed) {
					putchar(' ');
				}

				fputs(command.argv[i], stdout);
				printed = true;
			}
			/* End the echoed line when at least one argument was printed */
			if (printed) {
				putchar('\n');
			}

			/* Echo is handled directly by the shell, so skip fork/exec */
			continue;
		}
		else if (strcmp(command.argv[0], "X") == 0) {

			/* X must be followed by a program name to execute */
			if (command.argc < 2 || command.argv[1] == NULL || command.argv[1][0] == '\0') {
				printf("X: missing program name\n");
				continue;
			}

			/*
			 * Shift argv left by one slot to remove X.
			 * Before: argv[0]="X", argv[1]="prog", argv[2]="arg1", ...
			 * After:  argv[0]="prog", argv[1]="arg1", ...
			 */
			for (int i = 0; i < command.argc - 1; i++) {
				command.argv[i] = command.argv[i + 1];
			}

			/* Decrease argc to match the shortened argument list */
			command.argc--;

			/* Null-terminate argv for execvp */
			command.argv[command.argc] = NULL;

			/* Update command name to the real program being invoked */
			command.name = command.argv[0];
		}
		else if (strcmp(command.argv[0], "L") == 0) {

			/* Print a blank line before the location/listing output */
			putchar('\n');
			fflush(stdout);

			/* Run pwd in a child process and wait for it to finish */
			pid = fork();
			if (pid == 0) {
				char *pwd_args[] = { "pwd", NULL };
				execvp("pwd", pwd_args);
				perror("execvp");
				exit(1);
			}
			waitpid(pid, &status, 0);

			/* Separate pwd output from ls output with another blank line */
			putchar('\n');
			fflush(stdout);

			/* Run ls -l in a child process and wait for completion */
			pid = fork();
			if (pid == 0) {
				char *ls_args[] = { "ls", "-l", NULL };
				execvp("ls", ls_args);
				perror("execvp");
				exit(1);
			}
			waitpid(pid, &status, 0);

			/* L is fully handled here; skip the generic fork/exec block */
			continue;
		}
		/* H displays the help manual directly in the shell */
		else if (strcmp(command.argv[0], "H") == 0) {
			printHelp();
			continue;
		}
	  
		/* Create a child process to execute the command */
	    if ((pid = fork()) == 0) {
	       /* Child executing command */
	       execvp(command.name, command.argv);
		   perror("execvp");   /* print an error message if execvp fails */
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

/* Help (--help) function */

/* Displays the user manual for this shell. */
void printHelp() {
printf("\n");
    printf("=====================================================\n");
    printf("          linux (ce90)|> Shell User Manual          \n");
    printf("=====================================================\n");

    /* Overview */
    printf("\nOVERVIEW\n");
    printf("  This is a simple command-line shell for Linux. When you see\n");
    printf("  the prompt  linux (ce90)|>  the shell is ready to accept a\n");
    printf("  command. Type a command and press Enter to run it. The shell\n");
    printf("  will carry out the command and then show the prompt again.\n");
    printf("  Only one command may be entered per line.\n");

    printf("  To exit the shell at any time, use the Q command.\n");

    /* General usage */
    printf("\nUSAGE\n");
    printf("  linux (ce90)|> COMMAND [arguments]\n");
    printf("\n");
    printf("  This shell supports two kinds of commands:\n");
    printf("    1. Built-in shorthand commands (listed below).\n");
    printf("    2. Any standard Linux program installed on the system,\n");
    printf("       such as grep, cat, or gcc. Simply type the program\n");
    printf("       name and any arguments as you normally would.\n");
	printf("  	3. Pressing Enter without typing a command will simply\n");
	printf("  	   display a new prompt.\n");
    printf("\n");
	printf("  NOTE: Built-in commands must be typed as capital letters.\n");
	printf("  External Linux commands (such as ls or grep) follow normal\n");
	printf("  Linux case rules.\n");

    /* Command descriptions */
    printf("\nCOMMANDS\n");

    printf("\n  C file1 file2  -- Copy\n");
    printf("    Creates a copy of file1 named file2. file1 is not deleted.\n");
    printf("    Both file names are required.\n");
    printf("    Example:  C report.txt report_backup.txt\n");

    printf("\n  D file         -- Delete\n");
    printf("    Permanently deletes the named file. Use with caution;\n");
    printf("    deleted files cannot be recovered.\n");
    printf("    Example:  D oldfile.txt\n");

    printf("\n  E comment      -- Echo\n");
    printf("    Prints the given text to the screen followed by a new line.\n");
    printf("    Multiple spaces or tabs between words will be collapsed to\n");
    printf("    a single space. If no text is provided, a new prompt is shown.\n");
    printf("    Example:  E Hello World\n");

    printf("\n  H              -- Help\n");
    printf("    Displays this user manual. No arguments are needed.\n");
    printf("    Example:  H\n");

    printf("\n  L              -- List\n");
    printf("    Displays the current directory path followed by a detailed\n");
    printf("    listing of all files and folders in that directory. The\n");
    printf("    listing shows file permissions, owner, size, and last\n");
    printf("    modified date for each item.\n");
    printf("    Example:  L\n");

    printf("\n  M file         -- Make\n");
    printf("    Opens the named file in the nano text editor so you can\n");
    printf("    create or edit it. If the file does not exist it will be\n");
    printf("    created when you save. To save your work press Ctrl+O,\n");
    printf("    then Enter to confirm. To exit nano press Ctrl+X.\n");
    printf("    Example:  M notes.txt\n");

    printf("\n  P file         -- Print\n");
    printf("    Displays the contents of the named file on the screen one\n");
    printf("    page at a time. Press Space to advance to the next page\n");
    printf("    or press Q to stop viewing early.\n");
    printf("    Example:  P notes.txt\n");

    printf("\n  Q              -- Quit\n");
    printf("    Exits the shell and returns to the system. Any programs\n");
    printf("    launched during the session will have already finished\n");
    printf("    before this point.\n");
    printf("    Example:  Q\n");

    printf("\n  W              -- Wipe\n");
    printf("    Clears all text from the screen. The prompt will reappear\n");
    printf("    at the top. Your files and work are not affected.\n");
    printf("    Example:  W\n");

    printf("\n  X program      -- Execute\n");
    printf("    Runs the named program. This is useful for running programs\n");
    printf("    in the current directory or elsewhere on the system. Any\n");
    printf("    additional arguments after the program name will be passed\n");
    printf("    to the program.\n");
    printf("    Example:  X myprogram\n");
    printf("    Example:  X myprogram arg1 arg2\n");
	printf("    If the program is in the current directory, prefix it with\n");
	printf("    ./ (example: X ./myprogram).\n");

    /* Error behavior */
	printf("\nERRORS\n");
	printf("  If you type an unrecognized command, the shell will attempt\n");
	printf("  to run it as a standard Linux program. If no such program\n");
	printf("  exists, an error message will be displayed and the prompt\n");
	printf("  will return.\n");
	printf("  If a command is given invalid or missing arguments, the\n");
	printf("  underlying Linux program will display an appropriate error\n");
	printf("  message. For example, typing C with only one file name will\n");
	printf("  produce an error from cp describing the correct usage.\n");

    printf("\n=====================================================\n");
    printf("\n");
}

/* End help function */

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
    /* force the prompt to display immediately (don't wait for newline) */
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
