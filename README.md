# Linux Shell Project

## 1. How to Compile and Run

### Prerequisites
- Linux environment
- `gcc` compiler
- Standard Unix utilities used by this shell (`cp`, `rm`, `more`, `clear`, `nano`, `pwd`, `ls`)

### Build Steps
1. Open a terminal in the project directory:
   ```bash
   cd <directory containing shell.c>
   ```
2. Compile the source file:
   ```bash
   gcc -Wall -Wextra -o shell shell.c
   ```
3. (Optional) Verify the executable exists:
   ```bash
   ls -l shell
   ```

### Run Steps
1. Start the shell:
   ```bash
   ./shell
   ```
2. You should see the prompt:
   ```
   linux (ce90)|>
   ```
3. Enter commands (examples are in Section 2).
4. Exit the shell with:
   ```
   Q
   ```

## 2. How to Use This Shell

The shell accepts one command per line at the prompt `linux (ce90)|>`.

### Built-in Commands (uppercase)
- `C file1 file2`: Copy `file1` to `file2` (mapped to `cp`)
- `D file`: Delete `file` (mapped to `rm`)
- `E text...`: Echo text to the terminal (handled directly in shell)
- `H`: Show help manual
- `L`: Print working directory, then run `ls -l` to list directory contents
- `M file`: Open/create `file` in `nano`
- `P file`: Display file with pager (mapped to `more`)
- `Q`: Quit shell
- `W`: Clear terminal (mapped to `clear`)
- `X program [args...]`: Execute a program with optional arguments

### External Linux Commands
If the command is not one of the built-ins above, the shell tries to execute it as a normal Linux command using `execvp`.

Examples:
```bash
linux (ce90)|> ls
linux (ce90)|> grep main shell.c
linux (ce90)|> X ./myprogram arg1 arg2
```

## 3. Functions in This Shell

### User-defined functions in `shell.c`
- `int main(int argc, char *argv[])`
  - Main command loop; handles built-ins, process creation, and command dispatch.
- `void printHelp()`
  - Prints the shell user manual.
- `int parseCommand(char *cLine, struct command_t *cmd)`
  - Tokenizes user input and builds `argc/argv` for execution.
- `void printPrompt()`
  - Prints the shell prompt and flushes output.
- `void readCommand(char *buffer)`
  - Reads one input line from `stdin` using `fgets`.

### Key system/library functions used
- Process control: `fork`, `execvp`, `waitpid`, `exit`
- Input/output: `printf`, `putchar`, `fputs`, `fflush`, `fgets`, `perror`
- String handling/parsing: `strcmp`, `strcpy`, `strlen`, `strsep`
- Memory allocation: `malloc`

## 4. Citations / Resources Used

1. Nutt, G. J. *Operating Systems: A Modern Perspective* (course textbook).
   - Used for shell structure concepts (prompt/read/parse/execute cycle) and process-management background.
2. Course lecture/lab materials for CS 3460/426 (Lab 1: Basic C Shell).
   - Used as the assignment framework and baseline shell workflow.
3. Linux manual pages:
   - `fork(2)`: https://man7.org/linux/man-pages/man2/fork.2.html
   - `execvp(3)`: https://man7.org/linux/man-pages/man3/exec.3.html
   - `waitpid(2)`: https://man7.org/linux/man-pages/man2/waitpid.2.html
   - `strsep(3)`: https://man7.org/linux/man-pages/man3/strsep.3.html

