/*
 * =====================================================================================
 *
 *       Filename:  xibugger.c
 *
 *    Description:  a simple debugger, a toy, a demo.
 *
 *        Version:  1.0
 *        Created:  07/17/2014 09:42:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int procprint(const char *format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    return 0;// return count of printed char maybe better
}

void run_target(const char *name)
{
    procprint("child start!\n");
    if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0){
        perror("error ptrace()\n");
    }
    execl(name, name, NULL);
}

struct xibugger_breakpoint{
    int addr;
    int ins;//original instruction
};

struct xibugger_breakpoint *get_breakpoints(pid_t pid)
{
    struct xibugger_breakpoint *bp = (struct xibugger_breakpoint*)malloc(sizeof(struct xibugger_breakpoint));
    assert(bp!=NULL);
    // dirty hack
    bp->addr = 0x804843C;
    // now, get origanl
    bp->ins  = ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0);
    procprint("target orig instruction:%08X\n", bp->ins);
    
    // replace & show
    ptrace(PTRACE_POKETEXT, pid, (void*)bp->addr, (bp->ins&0xFFFFFF00)|0xCC);
    procprint("target new  instruction:%08X\n", ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0));
    return bp;
}

void execute_beforebreak(pid_t pid, struct user_regs_struct *regs,  struct xibugger_breakpoint *bps)
{
    int status;
    ptrace(PTRACE_POKETEXT, pid, (void*)bps->addr, bps->ins);
    ptrace(PTRACE_GETREGS, pid, 0, regs);
    regs->eip = bps->addr;// reset to orignal addr
    ptrace(PTRACE_SETREGS, pid, 0, regs);
    if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0){
    //if(ptrace(PTRACE_CONT, pid, 0, 0) < 0){
        perror("error execute_beforebreak:ptrace()\n");
        return ;
    }
    // wait for execute orignal instruction at breakpoint
    wait(&status);
    if(WIFEXITED(status)){
        procprint("target exited!\n");
        return ;
    }
}

void restore_breakpoints(pid_t pid, struct xibugger_breakpoint *bps)
{
    ptrace(PTRACE_POKETEXT, pid, (void*)bps->addr, (bps->ins&0xFFFFFF00)|0xCC);
}

void run_debugger(pid_t pid)
{
    int status;
    int count=0;
    struct user_regs_struct regs;

    // wait target stoped at 1st instruction
    wait(&status);

    // get breakpoint, support only 1 breakpoint now. Oops
    struct xibugger_breakpoint *breakpoints = get_breakpoints(pid);    

    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    procprint("target start at : %08X\n", regs.eip);

    // wait for target stop at breakpoint
    ptrace(PTRACE_CONT, pid, 0, 0);
    wait(0);
    
    while(WIFSTOPPED(status)){
        procprint("");

        while(getchar()!='\n'){
            ;
        }
    
        // we replace the 1st bytes of orignal instruction into '0xCC'
        // now, we retore it back and execute the orignal instruction.
        execute_beforebreak(pid, &regs, breakpoints);

        count++;
        
        // retore the breakpoint
        // breakpoint is always valid unless user delete it.
        restore_breakpoints(pid, breakpoints);


        // target continue, wait for next come across breakpoint or exited
        ptrace(PTRACE_CONT, pid, 0, 0);
        wait(&status);
    }
    procprint("child executed %d instructions\n", count);
}


int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("usage:%s target\n", argv[0]);
        return -1;
    }
    pid_t pid = fork();
    if( pid == 0 ){
        // child(target)
        run_target(argv[1]);
    }
    else if (pid > 0){
        // parent(debugger)
        run_debugger(pid);
    }
    else{
        perror("error fork()\n");
    }

    return 0;
}
