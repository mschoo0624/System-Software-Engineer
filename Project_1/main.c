#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
// ============================================
// TODO 1: COMMAND PARSING
// ============================================
// Build a function to parse user input into command and arguments
// Should handle:
//   - Command name
//   - Arguments
//   - Input/Output redirection (< and >)
//   - Pipes (|)
//   - Background execution (&)
//
// Function signature suggestion:
typedef struct {
    char *args[MAX_ARGS];
    char *input_file;    // for < redirection
    char *output_file;   // for > or >> redirection
    int output_append;   // 1 if >> used
    int background;      // 1 if & at end
} Command;

// Helper Function. (stdin <) 
void redirect_input(const char* filename){
    // Return if filename is NULL.
    if (!filename) return;
    // Open the file for reading the inputs. 
    int fd = open(filename, O_RDONLY, O_TRUNC, 0644);
    if (fd == -1){
        perror("open input");
        exit(EXIT_FAILURE); // child should exit on failure
    }
    // Duplicate the file descriptor to stdin.
    if (dup2(fd, STDIN_FILENO) == -1){
        perror("dup2 input");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // It replaces the current process image with a new program image.
    close(fd);
}
// ============================================
// TODO 2: PROCESS CREATION & EXECUTION
// ============================================
// Build a function to:
//   - Use fork() to create a child process
//   - Use execvp() to execute the command
//   - Handle errors if command not found
//
// Function signature suggestion:
// int execute_command(Command *cmd) {
//     pid_t pid = fork();
//     if (pid == 0) {
//         // Child process
//         // TODO: Handle input/output redirection
//         // TODO: Execute command with execvp()
//     } else if (pid > 0) {
//         // Parent process
//         // TODO: Wait for child if not background
//     } else {
//         // TODO: Handle fork error
//     }
// }

// ============================================
// TODO 3: INPUT/OUTPUT REDIRECTION
// ============================================
// Functions to handle:
//   - Input redirection: cmd < input.txt
//   - Output redirection: cmd > output.txt
//   - Append redirection: cmd >> output.txt
//
// Use open(), dup2(), and close() system calls
// These functions are intended to be called in the child process before execvp().

// Redirect stdin from filename. On failure, prints an error and exits the child.
void redirect_input(const char *filename) {
    if (!filename) return;
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open input");
        exit(EXIT_FAILURE); // child should exit on failure
    }
    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2 input");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// Redirect stdout to filename. If append != 0 use O_APPEND (>>), otherwise truncate (>).
void redirect_output(const char *filename, int append) {
    if (!filename) return;
    int flags = O_WRONLY | O_CREAT;
    flags |= append ? O_APPEND : O_TRUNC;
    int fd = open(filename, flags, 0644);
    if (fd < 0) {
        perror("open output");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2 output");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// ============================================
// TODO 4: PIPE HANDLING
// ============================================
// Build a function to handle piped commands: cmd1 | cmd2
//
// Steps:
//   1. Detect pipe in command
//   2. Create a pipe with pipe() system call
//   3. Fork two processes
//   4. Connect stdout of first process to pipe
//   5. Connect stdin of second process to pipe
//
// Function signature suggestion:
// int execute_pipe(Command *cmd1, Command *cmd2) {
//     int pipefd[2];
//     // TODO: Create pipe, fork processes, execute both
// }

// ============================================
// TODO 5: SIGNAL HANDLING
// ============================================
// Build a signal handler for:
//   - SIGINT (Ctrl+C): Should terminate current command, not shell
//   - SIGCHLD: Handle background process termination
//
// Function signature suggestion:
// void signal_handler(int sig) {
//     // TODO: Handle SIGINT or SIGCHLD
// }

// ============================================
// TODO 6: JOB CONTROL (Background Processes)
// ============================================
// Build structures and functions to:
//   - Track background processes
//   - Display running jobs with: jobs command
//   - Bring jobs to foreground: fg %1
//
// Suggested data structure:
// typedef struct {
//     pid_t pid;
//     char *command;
//     int status;  // 1=running, 0=stopped
// } Job;

// ============================================
// TODO 7: BUILT-IN COMMANDS
// ============================================
// Implement these built-in commands (don't require fork/exec):
//   - cd <directory>    : Change directory
//   - exit              : Exit shell
//   - pwd               : Print working directory
//   - echo <text>       : Print text
//   - jobs              : List background jobs
//   - fg %n             : Bring job to foreground
//
// Function signature suggestion:
// int is_builtin(const char *cmd) {
//     // TODO: Check if command is built-in
// }
//
// int execute_builtin(Command *cmd) {
//     // TODO: Execute built-in command
// }

// ============================================
// MAIN SHELL LOOP
// ============================================
int main() {
    char input[MAX_INPUT];
    
    // TODO 5: Register signal handlers
    // signal(SIGINT, signal_handler);
    // signal(SIGCHLD, signal_handler);
    
    while (1) {
        printf("myshell> ");
        fflush(stdout);
        
        // Read user input
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            perror("fgets");
            continue;
        }
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) continue;
        
        // TODO 1: Parse command
        // Command *cmd = parse_command(input);
        
        // TODO 7: Check if built-in command
        // if (is_builtin(cmd->args[0])) {
        //     execute_builtin(cmd);
        // } else {
        //     // TODO 2: Execute external command
        //     // execute_command(cmd);
        // }
        
        printf("Command: %s\n", input);  // Placeholder
    }
    
    return 0;
}

// ============================================
// IMPLEMENTATION CHECKLIST
// ============================================
// [ ] 1. Command parsing function
// [ ] 2. Process creation and execution (fork/exec)
// [ ] 3. Input/Output redirection (< and >)
// [ ] 4. Pipe handling (|)
// [ ] 5. Signal handlers (SIGINT, SIGCHLD)
// [ ] 6. Job control (background processes)
// [ ] 7. Built-in commands (cd, exit, pwd, etc.)
// [ ] 8. Main shell loop refinement
// [ ] 9. Error handling throughout
// [ ] 10. Testing with various commands
