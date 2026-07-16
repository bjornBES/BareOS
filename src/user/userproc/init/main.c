/*
 * File: main.c
 * File Created: 28 Apr 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 02 Jul 2026
 * Modified By: BjornBEs
 * -----
 */

#include "unistd.h"
#include "stdio.h"
#include "signal.h"
#include "sys/wait.h"

uint8_t test = 0;

void signal_abort(int signum)
{
    fprintf(stddebug, "hello got signal %d\n", signum);
}

int main(int argc, char *argv[])
{
    pledge(PLEDGE_STDIO | PLEDGE_MALLOC | PLEDGE_PROC | PLEDGE_EXEC);
    write(stdout, "hello from C\n", 13);
    
    fprintf(stddebug, "argv @ %p\n", argv);
    fprintf(stddebug, "argc = %u\n", argc);
    fprintf(stddebug, "got args:\n");
    for (int i = 0; i < argc; i++)
    {
        fprintf(stddebug, "\t[%i] = \"%s\" @ %p\n", i, argv[i], argv[i]);
    }
    
    /*     void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    fprintf(stddebug, "hello %p\n", addr); */
    
    pid_t curr_proc = getpid();
    fprintf(stddebug, "process is %u\n", curr_proc);
    
    sigaction(SIGABRT, signal_abort);
    kill(curr_proc, SIGABRT);                        // or however you send signals to self
    fprintf(stddebug, "back from signal handler\n"); // if this prints, it worked
    
    pid_t child = fork();
    if (child == 0)
    {
        pledge(PLEDGE_STDIO | PLEDGE_MALLOC | PLEDGE_PROC | PLEDGE_EXEC);
    }
    fprintf(stddebug, "X got %u back from fork\n", child);
    if (child == 0)
    {
        pid_t child_proc = getpid();
        fprintf(stddebug, "running bash on %u/%u\n", child_proc, child);
        const char *argv[] = {"bin/ash", NULL};
        const char *envp[] = {"PATH=/bin:/usr/bin", "HOME=/", "TERM=linux", NULL};
        execve("bin/ash", argv, envp);
        kill(child_proc, SIGABRT);
        _exit(1);
    }

    fprintf(stddebug, "waiting for pid%u to exit\n", child);
    waitpid(child, NULL, 0);
    fprintf(stddebug, "enter loop\n");

    while (true)
    {
        /* code */
    }

    return 0;
}