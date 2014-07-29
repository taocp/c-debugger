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
#include <string.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#include "lib/datastruct.h"
#include "dwarf/dwarf.h"

#define XIBUGGER_CMD_LEN 10

struct xibugger_breakpoint{
    int addr;// the address of breakpoint
    int ins; //original instruction, we change it's 1st byte into '0xCC'
};

// http://stackoverflow.com/questions/2352209/max-identifier-length
#define FUNC_NAME_LEN 63

struct func_name_addr{
    char name[FUNC_NAME_LEN];
    int  addr;
};

//  just like printf(), but add a header: [pid]
int procprint(const char *format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    return 0;// return count of printed char maybe better
}

//   debugger wait target stopped
void xibugger_wait(pid_t pid)
{
    int status;
    wait(&status);
    if( !WIFSTOPPED(status) ){
        procprint("target exited\n");
        exit(0);
    }
}

void run_target(const char *name)
{
    if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0){
        perror("error ptrace()\n");
    }
    if( execl(name, name, NULL) == -1 ){
        procprint("cannot execute the target\n");
    }
}

//  debugger get targe's eip
int get_eip(pid_t pid)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    return regs.eip;
}

// debugger get target's instruction with specified addr
inline int get_instruction(pid_t pid, int addr)
{
    return ptrace(PTRACE_PEEKTEXT, pid, (void*)addr, 0);
}

// TODO support multiple breakpoint
struct xibugger_breakpoint *get_breakpoints(pid_t pid)
{
    struct xibugger_breakpoint *bp = (struct xibugger_breakpoint*)malloc(sizeof(struct xibugger_breakpoint));
    assert(bp!=NULL);

    scanf("%x", &bp->addr);
    // TODO check input

    bp->ins  = ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0);
    procprint("target orignal instruction:%08X\n", bp->ins);
    
    ptrace(PTRACE_POKETEXT, pid, (void*)bp->addr, (bp->ins&0xFFFFFF00)|0xCC);
    procprint("target new  instruction:%08X\n", ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0));
    return bp;
}

void proc_ins_eip(pid_t pid)
{
    procprint(
            "current ins:%x, current eip:%x\n", 
            get_instruction(pid, get_eip(pid)),
            get_eip(pid)
    );
}

void execute_singlestep(pid_t pid)
{
//    proc_ins_eip(pid);
    if(ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0){
        perror("error execute_singlestep:ptrace()\n");
        return ;
    }
    xibugger_wait(pid);
//    proc_ins_eip(pid);
}

/*
 * example
 * the orignal instruction is : 0xcd80
 * debugger set a breakpoint on it, so it became into : 0xcc80
 * 
 * this function change 0xcc80 into 0xcd80
 * and execute the orignal instruction 0xcd80
 * at the end, change 0xcd80 into 0xcc80
 */
void dance_on_breakpoint(pid_t pid, struct user_regs_struct *regs, struct xibugger_breakpoint *bps)
{
    // we had replaced the 1st bytes of orignal instruction into '0xCC'
    // now, we retore it back and execute the orignal instruction.
    ptrace(PTRACE_POKETEXT, pid, (void*)bps->addr, bps->ins);
    ptrace(PTRACE_GETREGS, pid, 0, regs);
    regs->eip -= 1;
    ptrace(PTRACE_SETREGS, pid, 0, regs);
    execute_singlestep(pid);

    // retore the breakpoint
    // breakpoint is always valid unless user delete it.
    ptrace(PTRACE_POKETEXT, pid, (void*)bps->addr, (bps->ins&0xFFFFFF00)|0xCC);
}

void eatendline(void)
{
    while(getchar()!='\n'){
        ;
    }
}

void run_debugger(pid_t pid)
{
    struct user_regs_struct regs;

    // wait target stoped at 1st instruction
    xibugger_wait(pid);

    struct xibugger_breakpoint *breakpoints;    

    ptrace(PTRACE_GETREGS, pid, 0, &regs);
//    procprint("target start at : %08X\n", regs.eip);

    int  why_stop = 0;// why_stop == 0: come across breakpoint
                      // why_stop == 1: single step (press `n`)

    char cmd[XIBUGGER_CMD_LEN]="";
    int  is_running = 0; // The program is not being run.
    int  is_hadbreakpoint = 0;
    do{
        procprint("");

        // TODO just press ENTER repeat last operate
        if( EOF == scanf("%s", cmd) ){
            procprint("over\n");
            break;
        }
        if( !strcmp(cmd, "b") ){
            if(is_hadbreakpoint == 1){
                procprint("support only 1 breakpoint now.\n");
                eatendline();
                continue;
            }
            // get breakpoint, support only 1 breakpoint now. Oops
            breakpoints = get_breakpoints(pid);
            is_hadbreakpoint = 1;
            eatendline();
            continue; // get next cmd
        }
        else if( !strcmp(cmd, "p") ){
            proc_ins_eip(pid);
        }
        else if( !strcmp(cmd, "c") ){
            if( is_running != 1 ){
                procprint("The program is not being run.\n");
                eatendline();
                continue;
            }
            if(why_stop == 0){
                dance_on_breakpoint(pid, &regs, breakpoints);
            }
            ptrace(PTRACE_CONT, pid, 0, 0);
            xibugger_wait(pid);
            why_stop = 0;
            //procprint("current ins:%x\n", get_instruction(pid, breakpoints->addr));
        }
        else if( !strcmp(cmd, "n") ){
            if( is_running != 1 ){
                procprint("The program is not being run.\n");
                eatendline();
                continue;
            }
            if(why_stop == 0){
                dance_on_breakpoint(pid, &regs, breakpoints);
                why_stop = 1;
            }
            else{
                execute_singlestep(pid);
            }
        }
        else if( !strcmp(cmd, "r") ){
            if(is_running == 1){
                procprint("target is being run\n");
                eatendline();
                continue;
            }
            is_running = 1;
            ptrace(PTRACE_CONT, pid, 0, 0);
            xibugger_wait(pid);
        }
        else {
            procprint("no support for this operation\n");
            eatendline();
            continue;
        }
        eatendline();

    }while(1);
}

struct list_node *func_addr(char *target)
{
    assert(target);
    char filename[FUNC_NAME_LEN];
    FILE *fp;
    strncpy(filename, target, FUNC_NAME_LEN - strlen(suffix) - 1);
    strncat(filename, suffix, strlen(suffix));
    if ((fp = fopen(filename, "r")) < 0){
        return NULL;
    }
    char fun_name[FUNC_NAME_LEN];
    int  addr;
    struct func_name_addr *pdata = NULL;
    struct list_node *h=NULL;
    while( EOF != fscanf(fp, "%s%x", fun_name, &addr) ){
        pdata = (struct func_name_addr*)malloc(sizeof(struct func_name_addr));
        assert(pdata);
        strncpy(pdata->name, fun_name, FUNC_NAME_LEN);
        pdata->addr = addr;
        list_add(&h, pdata);
    }

    struct list_node *s=h;
    while(s){
        printf(
                "%s : %x\n",
                ((struct func_name_addr*)s->pdata)->name,
                ((struct func_name_addr*)s->pdata)->addr
        );
        s = s->next;
    }
    return h;
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
        char *dwarf_argv[] = { "dwarf", argv[1] };
        if( dwarf(2, dwarf_argv) != 0 ){
            perror("cannot read debug infos.\ncompile with -g option?\n");
            return -1;
        }
        struct list_node *func_list = func_addr(argv[1]);
        assert(func_list);
        run_debugger(pid);
    }
    else{
        perror("error fork()\n");
    }

    return 0;
}
