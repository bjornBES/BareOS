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
#include "sched.h"
#include "sys/wait.h"

uint8_t test = 0;

void signal_abort(int signum)
{
    pid_t pid = getpid();
    fprintf(stddebug, "hello got signal %d from pid %u\n", signum, pid);
}

void signal_child(int signum)
{
    fprintf(stddebug, "hello got signal %d\n", signum);
    fprintf(stddebug, "something happened to a child\n");
}

int main(int argc, char *argv[])
{
    pledge(PLEDGE_STDIO | PLEDGE_RPATH | PLEDGE_WPATH | PLEDGE_PROC | PLEDGE_MALLOC | PLEDGE_EXEC);
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

    sigaction_t action;
    action.handler.sa_handler = signal_abort;
    action.sa_flags = 0;
    action.sa_mask = 0;
    rt_sig_action(SIGABRT, &action, NULL);
    
    pid_t curr_proc = getpid();
    fprintf(stddebug, "process is %u\n", curr_proc);
    
    pid_t child = fork();
    if (child == 0)
    {
        pledge(0xFFFF);
    }
    fprintf(stddebug, "X got %u back from fork\n", child);
    if (child == 0)
    {
        pid_t child_proc = getpid();
        // fprintf(stddebug, "running bash on %u/%u\n", child_proc, child);
        // const char *argv[] = {"/user!/bin/ash", NULL};
        // const char *envp[] = {"PATH=/user!/bin:/user!/usr/bin", "HOME=/", "TERM=linux", NULL};
        // execve("bin/ash", argv, envp);
        kill(child_proc, SIGABRT);
        if (kill(curr_proc, SIGCHLD) != 0)
        {
            fprintf(stddebug, "Kill failed\n");
        }
        while (true)
        {
            /* code */
        }
        // _exit(1);
    }
    
    sigaction_t action_chld;
    action_chld.handler.sa_handler = signal_child;
    action_chld.sa_flags = 0;
    action_chld.sa_mask = 0;
    rt_sig_action(SIGCHLD, &action_chld, NULL);

    // fprintf(stddebug, "waiting for pid%u to exit\n", child);
    sched_yield();
    fprintf(stddebug, "enter loop\n");

    while (true)
    {
        /* code */
    }

    return 0;
}
