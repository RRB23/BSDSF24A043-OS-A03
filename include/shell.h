#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 50
#define ARGLEN 100
#define HISTORY_SIZE 20
#define MAX_JOBS 100
#define PROMPT "FCIT> "

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
    int active;
} Job;

extern Job jobs[MAX_JOBS];

char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background);
int handle_builtin(char** arglist);
void reap_jobs(void);
void add_job(pid_t pid, char* command);
void show_jobs(void);

#endif
