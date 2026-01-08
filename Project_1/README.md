# Mini Shell (Project_1)

A simple Unix-like shell implemented in C for learning system programming concepts: parsing, process creation (fork/exec), I/O redirection, pipes, background jobs, and basic built-ins.

## Features

- Command parsing (arguments, `<`, `>`, `>>`, `|`, `&`)  
- Parser implementation (tokenizer and pipeline splitter) in `main.c`  
- Makefile to build the program (`make`)  

> Note: This project currently includes a parsing implementation (TODO 1). Execution, redirection, pipes, signal handling and job control are TODOs.

## Files

- `main.c` — main program and TODO stubs (parser implemented)  
- `makefile` — build rules

## Build

```sh
cd Project_1
make
```

Or build manually:

```sh
gcc -o main main.c -Wall -Wextra -std=c99
```

## Run

```sh
./main
```

You should see a prompt `myshell>`; enter commands for parsing tests.

## How to test the parser

1. Run `./main`.
2. Enter commands such as:
   - `ls -l`
   - `echo hello > out.txt`
   - `cat < in.txt | grep foo > out.txt &`
3. The current placeholder prints parsed segments, arguments, input/output file names, append flag and background flag.

## Limitations / Notes

- The tokenizer is **not quote-aware**: arguments with spaces inside quotes are not currently supported.
- The project still needs implementations for:
  - Process creation and exec (`fork`/`execvp`) — TODO 2
  - Input/output redirection (`dup2` + `open`) — TODO 3
  - Pipe execution (multiple processes connected by `pipe`) — TODO 4
  - Signal handling (SIGINT/SIGCHLD) — TODO 5
  - Job control and built-ins (`cd`, `jobs`, `fg`, etc.) — TODO 6/7

## Next steps (suggested)

1. Implement `redirect_input` and `redirect_output` helpers (use `open`, `dup2`).
2. Implement `execute_command` (handle background jobs and builtin checks).
3. Implement `execute_pipe` for pipelines.
4. Add signal handlers and job table for background processes.
