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
#define MAX_JOBS 100
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
    int fd = open(filename, O_RDONLY, 0644);
    if (fd == -1){
        perror("open input");
        exit(EXIT_FAILURE); // child should exit on failure
    }
    // Duplicate the file descriptor to stdin.
    if (dup2(fd, STDIN_FILENO) == -1){
        perror("dup1 input");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // It replaces the current process image with a new program image.
    close(fd);
}

// Helper Function. (stdout > or >>)
void redirect_output(const char* filename, int append){
    // Returning if filename is NULL;
    if (!filename) return;
    // Open the file for writing the outputs.
    // Using the TRUNC flag to truncate the file if it already exists(>). and APPEND flag to append the data to the file(>>).
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1){
        perror("open output");
        exit(EXIT_FAILURE); // child should exit on failure
    }

    if (dup2(fd, STDOUT_FILENO) == -1){
        perror("dup2 output");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// buidling a Parse Command Function.   
Command* parse_command(const char* input) {
    // Creating the Command struct pointer.
    Command *cmd = malloc(sizeof(Command));

    if (!cmd) return NULL;

    // Initializing the Command struct members.
    memset(cmd, 0, sizeof(Command));

    char *token;
    char *copied_line = strdup(input); // allocates sufficient memory for a copy of the string str 

    if (!copied_line) {
        perror("copied line.");
        free(cmd);
        return NULL;
    }

    int track = -1; // to track the arguments index.

    // Tokenizing the input string based on spaces.
    token = strtok(copied_line, " ");

    while (token != NULL) {
        if (strcmp(token, "<") == 0){
            token = strtok(NULL, " ");
            cmd->input_file = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            cmd->output_file = strdup(token);
            cmd->output_append = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            cmd->output_file = strdup(token);
            cmd->output_append = 1;
        } else if (strcmp(token, "&") == 0) {
            cmd->background = 1;
        } else {
            track++;
            cmd->args[track] = strdup(token);
        }
        token = strtok(NULL, " ");
    }

    cmd->args[track + 1] = NULL;
    free(copied_line);
    return cmd;
}

int is_builtin(const char *cmd) {
    if (!cmd) return 0;

    return strcmp(cmd, "cd") == 0 ||
           strcmp(cmd, "exit") == 0 ||
           strcmp(cmd, "pwd") == 0 ||
           strcmp(cmd, "echo") == 0 ||
           strcmp(cmd, "jobs") == 0 ||
           strcmp(cmd, "fg") == 0 ||
           strcmp(cmd, "bg") == 0;
}

int execute_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "cd") == 0) {
        if (!cmd->args[1]) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(cmd->args[1]) != 0) {
            perror("cd");
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[1024]; //  Current Working Directory buffer should be 1024bytes.  
        if (getcwd(cwd, sizeof(cwd)))
            printf("%s\n", cwd);
        else
            perror("pwd");
        return 1;
    }

    if (strcmp(cmd->args[0], "echo") == 0) {
        for (int i = 1; cmd->args[i]; i++) {
            printf("%s ", cmd->args[i]);
        }
        printf("\n");
        return 1;
    }

    if (strcmp(cmd->args[0], "jobs") == 0) {
        // TODO: Implement jobs command - list all background jobs
        printf("jobs: not implemented yet\n");
        return 1;
    }

    if (strcmp(cmd->args[0], "fg") == 0) {
        // TODO: Implement fg command - bring job to foreground
        if (!cmd->args[1]) {
            fprintf(stderr, "fg: missing job number\n");
        } else {
            printf("fg %s: not implemented yet\n", cmd->args[1]);
        }
        return 1;
    }

    if (strcmp(cmd->args[0], "bg") == 0) {
        // TODO: Implement bg command - resume job in background
        if (!cmd->args[1]) {
            fprintf(stderr, "bg: missing job number\n");
        } else {
            printf("bg %s: not implemented yet\n", cmd->args[1]);
        }
        return 1;
    }

    return 0;
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
int execute_command(Command *cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // TODO: Handle input/output redirection
        // TODO: Execute command with execvp()
        if (cmd->input_file){
            redirect_input(cmd->input_file);
        }
        if (cmd->output_file){
            redirect_output(cmd->output_file, cmd->output_append);
        }
        // Checking the absolute or relative path.
        if (strchr(cmd->args[0], '/')) {
            execv(cmd->args[0], cmd->args);
            perror("execv");
        } else {
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
        }
        
    } else if (pid > 0) {
        // Parent process
        // TODO: Wait for child if not background
        // TODO: If background, add to job list
        if (!cmd->background) {
            waitpid(pid, NULL, 0);
        } else {
            // TODO: add_job(pid, input, 0); // running status
        }
    } else {
        // TODO: Handle fork error
        perror("fork");
        return -1;
    }
    return pid;
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
int execute_pipe(char *input) {
    char *track = NULL;
    
    char *pipeline = strchr(input, '|');
    char *logicalOr = strstr(input, "||");

    if ((track = logicalOr) != NULL) {
        *track = '\0';
        Command *cmd1 = parse_command(input);
        pid_t pid1 = execute_command(cmd1);
        int status;
        waitpid(pid1, &status, 0);
        
        if (WEXITSTATUS(status) != 0) {
            Command *cmd2 = parse_command(track + 2); // Move past the "||"
            pid_t pid2 = fork();
            if (pid2 == 0) {
                execvp(cmd2->args[0], cmd2->args);
                exit(0);
            }
            waitpid(pid2, NULL, 0);
        }
        return 0;   
    }

    if ((track = pipeline) != NULL)  {
        *track = '\0';
        char *cmd1 = input;
        char *cmd2 = track + 1;  // Move past the "|"
        
        // Create a pipe.
        int pipefd[2];

        if (pipe(pipefd) == -1){
            perror("pipe");
            return -1;
        }

        pid_t pid1 = fork();
        if (pid1 == 0){
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            Command *first = parse_command(cmd1);
            execvp(first->args[0], first->args);
            exit(0);
        }

        pid_t pid2 = fork();
        if (pid2 == 0){
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            Command *second = parse_command(cmd2);
            execvp(second->args[0], second->args);
            exit(0);
        }
        // Parent Process. 
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 0;
    }

    Command *cmd = parse_command(input);
    
    int status;
    waitpid(execute_command(cmd), &status, 0);
    return WEXITSTATUS(status) ? WEXITSTATUS(status) : 1;
}

// ============================================
// TODO 5: SIGNAL HANDLING
// ============================================
// Build a signal handler for:
//   - SIGINT (Ctrl+C): Should terminate current command, not shell
//   - SIGCHLD: Handle background process termination
//   - SIGTSTP (Ctrl+Z): Suspend current foreground job
//   - SIGTTIN/SIGTTOU: Handle job control for background I/O
//
// Additional features to implement:
//   - Job control: Maintain a list of jobs (foreground/background)
//   - fg command: Bring background job to foreground
//   - bg command: Resume suspended job in background
//   - jobs command: List all jobs with status
//   - Proper process group management
//   - Terminal control (tcsetpgrp) for job switching
//
// Function signature suggestion:
void signal_handler(int sig) {
    // TODO: Handle SIGINT or SIGCHLD
    if (sig == SIGINT) {
        printf("\n");
    } else if (sig == SIGCHLD) {
        int status;
        pid_t pid;
        // WNOHANG = non-blocking (don't wait, return immediately).
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("Background process %d terminated.\n", pid);
        }
    }
    // TODO: Add SIGTSTP handler to suspend foreground job
    // TODO: Add job list management for background processes
    // TODO: Implement proper foreground/background process groups
}

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
//   - bg %n             : Resume job in background
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
// TODO 8: JOB CONTROL
// ============================================
// Implement job control system:
//   - Job struct: pid, command, status (running/stopped), job number
//   - Global job list array or linked list
//   - Add jobs to list when started in background or suspended
//   - Update job status in SIGCHLD handler
//   - Implement fg: find job by number, set as foreground, continue if stopped
//   - Implement bg: find job by number, continue in background
//   - Implement jobs: print all jobs with status
//   - Handle process groups and terminal control
//
// Function signatures suggestion:
typedef struct Job {
     pid_t pid;
     char *command;
     int status; // 0=running, 1=stopped, 2=done
     int job_num;
} Job;

Job *job_list[MAX_JOBS];
int next_job_num = 1;

void add_job(pid_t pid, char *cmd, int status) {
    if (next_job_num > MAX_JOBS) {
        fprintf(stderr, "Job limit reached.\n");
        return;
    }
    Job *job = malloc(sizeof(Job));
    job->pid = pid;
    job->command = strdup(cmd);
    job->status = status;
    job->job_num = next_job_num++;
    job_list[job->job_num] = job;

}
void remove_job(int job_num) {
    if (job_num <= 0 || job_num > MAX_JOBS || !job_list[job_num]) {
        fprintf(stderr, "Invalid job number.\n");
        return;
    }
    free(job_list[job_num]->command);
    free(job_list[job_num]);
    job_list[job_num] = NULL;
}
Job *find_job(int job_num) {
    if (job_num <= 0 || job_num > MAX_JOBS) {
        fprintf(stderr, "Invalid job number.\n");
        return NULL;
    }
    return job_list[job_num];
}
void print_jobs() {
    for (int i = 1; i < next_job_num; i++) {
        if (job_list[i]) {
            printf("[%d] %s (PID: %d) - %s\n", job_list[i]->job_num, job_list[i]->command, job_list[i]->pid, 
                job_list[i]->status == 0 ? "Running" : job_list[i]->status == 1 ? "Stopped" : "Done");
        }
    }
}
void fg_job(int job_num) {
    // 1. Find the job by job_num
    // 2. If job is stopped, send SIGCONT to resume it
    // 3. Set the job's process group as foreground
    // 4. Wait for the job to complete or be stoppe
    // 5. Restore shell as foreground process group
    // 6. Update job status based on how it ended
}
// void bg_job(int job_num);

// ============================================
// MAIN SHELL LOOP
// ============================================
int main() {
    char input[MAX_INPUT];
    
    // TODO 5: Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
    
    while (1) {
        printf("myshell> ");
        fflush(stdout);
        
        // Read user input
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;  // Exit on EOF
        }
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) continue;
        
        // TODO 1: Parse command
        Command *cmd = parse_command(input);
        
        if (!cmd) {
            fprintf(stderr, "parse returned NULL\n");
            continue;
        }
        
        fprintf(stderr, "DEBUG: cmd->args[0] = '%s'\n", cmd->args[0] ? cmd->args[0] : "NULL");
        
        if (!cmd->args[0]) {
            fprintf(stderr, "args[0] is NULL\n");
            continue;
        }
        
        // TODO 7: Check if built-in command
        if (is_builtin(cmd->args[0])) {
            execute_builtin(cmd);
        } else {
            // TODO 2: Execute external command
            // TODO: Update to handle job control for background processes
            execute_pipe(input);
        }
        
        // TODO: After execution, check for completed background jobs and update job list
        
        // Free allocated memory
        for (int i = 0; cmd->args[i]; i++) {
            free(cmd->args[i]);
        }
        if (cmd->input_file) free(cmd->input_file);
        if (cmd->output_file) free(cmd->output_file);
        free(cmd);
    }
    
    return 0;
}