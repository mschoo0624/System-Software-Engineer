#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s list\n", prog_name);
    printf("  %s todo <project>\n", prog_name);
    printf("  %s scaffold <project> <output-file>\n", prog_name);
    printf("\nProjects:\n");
    printf("  mini-shell          Shell with pipes and redirection\n");
    printf("  file-processor      Multithreaded text-search processor\n");
    printf("  tcp-server          Concurrent TCP server\n");
}

void show_project_todo(const char *project) {
    if (strcmp(project, "mini-shell") == 0) {
        printf("Mini shell TODO:\n");
        printf("1. Read a command line from stdin.\n");
        printf("2. Parse tokens, arguments, and separators: |, <, >, >>.\n");
        printf("3. Add built-in commands such as cd and exit.\n");
        printf("4. Launch external commands with fork() and execvp().\n");
        printf("5. Implement pipelines with pipe() and dup2().\n");
        printf("6. Support stdin/stdout redirection to files.\n");
        printf("7. Wait for foreground jobs with waitpid().\n");
    } else if (strcmp(project, "file-processor") == 0) {
        printf("File processor TODO:\n");
        printf("1. Parse command-line pattern and file paths.\n");
        printf("2. Create worker threads using pthreads.\n");
        printf("3. Build a thread-safe job queue or assign one file per thread.\n");
        printf("4. Open each file and scan lines for the pattern.\n");
        printf("5. Print matching lines with filename and line number.\n");
        printf("6. Use mutexes or condition variables to coordinate threads.\n");
        printf("7. Add optional flags for recursive scanning or case-insensitive search.\n");
    } else if (strcmp(project, "tcp-server") == 0) {
        printf("TCP server TODO:\n");
        printf("1. Create a listening socket with socket(), bind(), and listen().\n");
        printf("2. Accept new clients with accept().\n");
        printf("3. Handle each client concurrently with pthreads.\n");
        printf("4. Read client data and send responses.\n");
        printf("5. Close client sockets cleanly.\n");
        printf("6. Add graceful shutdown and error handling.\n");
    } else {
        printf("Unknown project '%s'.\n", project);
        show_usage("project_tool");
    }
}

int write_todo_scaffold(const char *path, const char *project) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }

    if (strcmp(project, "mini-shell") == 0) {
        fprintf(f, "/* mini-shell TODO scaffold */\n");
        fprintf(f, "/* 1. Read a line from stdin. */\n");
        fprintf(f, "/* 2. Tokenize arguments and operators: |, <, >, >>. */\n");
        fprintf(f, "/* 3. Implement builtins: cd, exit. */\n");
        fprintf(f, "/* 4. Use fork() + execvp() to run commands. */\n");
        fprintf(f, "/* 5. Use pipe() and dup2() for pipelines. */\n");
        fprintf(f, "/* 6. Redirect stdin/stdout to files. */\n");
        fprintf(f, "*/\n\n");
        fprintf(f, "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <unistd.h>\n#include <sys/wait.h>\n\nint main(void) {\n    char line[1024];\n    char *argv[64];\n    // TODO: implement command loop, parsing, builtins, forks, pipes, and redirects\n    return 0;\n}\n");
    } else if (strcmp(project, "file-processor") == 0) {
        fprintf(f, "/* file-processor TODO scaffold */\n");
        fprintf(f, "/* 1. Parse pattern and file list from argv. */\n");
        fprintf(f, "/* 2. Create pthread workers. */\n");
        fprintf(f, "/* 3. Build a thread-safe queue or assign files to threads. */\n");
        fprintf(f, "/* 4. Open files and search each line for the pattern. */\n");
        fprintf(f, "/* 5. Print matches with filename and line number. */\n");
        fprintf(f, "*/\n\n");
        fprintf(f, "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <pthread.h>\n\nint main(int argc, char *argv[]) {\n    // TODO: implement argument parsing, thread pool, and file scanning\n    return 0;\n}\n");
    } else if (strcmp(project, "tcp-server") == 0) {
        fprintf(f, "/* tcp-server TODO scaffold */\n");
        fprintf(f, "/* 1. Create listening socket and bind to a port. */\n");
        fprintf(f, "/* 2. Accept incoming connections. */\n");
        fprintf(f, "/* 3. Use pthreads to handle each client. */\n");
        fprintf(f, "/* 4. Read client requests and write responses. */\n");
        fprintf(f, "*/\n\n");
        fprintf(f, "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <unistd.h>\n#include <arpa/inet.h>\n#include <pthread.h>\n\nint main(void) {\n    // TODO: implement socket setup, accept loop, and client handler threads\n    return 0;\n}\n");
    } else {
        fprintf(stderr, "Unknown project '%s'.\n", project);
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "list") == 0) {
        show_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "todo") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s todo <project>\n", argv[0]);
            return EXIT_FAILURE;
        }
        show_project_todo(argv[2]);
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "scaffold") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s scaffold <project> <output-file>\n", argv[0]);
            return EXIT_FAILURE;
        }
        if (write_todo_scaffold(argv[3], argv[2]) != 0) {
            return EXIT_FAILURE;
        }
        printf("TODO scaffold written to %s\n", argv[3]);
        return EXIT_SUCCESS;
    }

    fprintf(stderr, "Unknown command '%s'.\n", argv[1]);
    show_usage(argv[0]);
    return EXIT_FAILURE;
}
