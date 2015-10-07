/*
 * =====================================================================================
 *
 *       Filename:  xibugger.c
 *
 *    Description:  a simple C debugger.
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
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dwarf/dwarf.h"
#include "lib/datastruct.h"

#define XIBUGGER_CMD_LEN 10

#define COME_ACROSS_BREAKPOINT 0

#define FUNC_ADDR(s) (((struct func_info_t*)s->pdata)->addr)

#define BREAKPOINT_ADDR(s) (((struct breakpoint_t*)s->pdata)->addr)

#define BREAKPOINT_INS(s) (((struct breakpoint_t*)s->pdata)->ins)

// http://stackoverflow.com/questions/2352209/max-identifier-length
#define FUNC_NAME_LEN 63

struct breakpoint_t{
    int addr;// the address of breakpoint
    int ins; //original instruction, we change it's 1st byte into '0xCC'
};

struct func_info_t{
    char name[FUNC_NAME_LEN];
    int  addr;
};

int search_func_byname(struct list_node *s, void *data);
int search_func_byaddr(struct list_node *s, void *data);
int search_bps(struct list_node *node, void *eip);
int traverse_bps(struct list_node *bp);

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

//  debugger get targe's eip
int get_eip(pid_t pid)
{
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    return regs.eip;
}

//   debugger wait target stopped
void xibugger_wait(pid_t pid)
{
    int status;
    wait(&status);
    if( WIFSTOPPED(status) ){
        int eip = get_eip(pid);
        if( eip == 0 ){
            procprint("target exited already.\n");
            exit(0);
        }
        return ;
    }
    if( WIFEXITED(status) ){
        procprint("target exited already.\n");
        exit(0);
    }
    assert(0);
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

// debugger get target's instruction with specified addr
inline int get_instruction(pid_t pid, int addr)
{
    return ptrace(PTRACE_PEEKTEXT, pid, (void*)addr, 0);
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
void dance_on_breakpoint(pid_t pid, struct breakpoint_t *bps)
{
    // we had replaced the 1st bytes of orignal instruction into '0xCC'
    // now, we retore it back and execute the orignal instruction.
    struct user_regs_struct regs;
    ptrace(PTRACE_POKETEXT, pid, (void*)bps->addr, bps->ins);
    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    regs.eip -= 1;
    ptrace(PTRACE_SETREGS, pid, 0, &regs);
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

// find the target stopped on which breakpoint when it break
struct breakpoint_t *which_breakpoint(pid_t pid, struct list_node *bps_list)
{
    int eip = get_eip(pid);
    eip -= 1; // had execute the instruction '0xCC', go back a step
    struct list_node *s = list_search(bps_list, search_bps, (void*)eip);
    assert(s);
    //procprint("%x : %x\n", ((struct breakpoint_t*)(s->pdata))->addr, ((struct breakpoint_t*)(s->pdata))->ins);
    return (struct breakpoint_t*)(s->pdata);
}

struct breakpoint_t *construct_breakpoint(pid_t pid, int addr)
{
    struct breakpoint_t *bp = (struct breakpoint_t*)malloc(sizeof(struct breakpoint_t));
    assert(bp!=NULL);
    bp->addr = addr;
    bp->ins  = ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0);
    //procprint("orignal instruction:%08X\n", bp->ins);
    ptrace(PTRACE_POKETEXT, pid, (void*)bp->addr, (bp->ins&0xFFFFFF00)|0xCC);
    //procprint("new     instruction:%08X\n", ptrace(PTRACE_PEEKTEXT, pid, (void*)bp->addr, 0));
    return bp;
}

void prompt_break(pid_t pid, struct list_node *bps_list, struct list_node *func_list)
{
    struct breakpoint_t *breakpoint = which_breakpoint(pid, bps_list);
    struct list_node *name_addr = list_search(func_list, search_func_byaddr, (void*)breakpoint->addr);
    assert(name_addr);
    procprint("breakpoint at : %s\n", ((struct func_info_t*)name_addr->pdata)->name);
}

int is_delete_bp(struct list_node *list, void *data)
{
    return ((struct breakpoint_t*)list->pdata)->addr == (int)data;
}

int is_breakpoint(struct list_node *list, void *data)
{
    return ((struct breakpoint_t*)list->pdata)->addr == (int)data;
}

void cmd_set_breakpoint(pid_t pid, struct list_node **bps_list, struct list_node *func_list)
{
    char name[FUNC_NAME_LEN];
    scanf("%s", name);
    struct list_node *func = list_search(func_list, search_func_byname, (void*)name);
    if(func){
        struct breakpoint_t *bp = construct_breakpoint(pid, FUNC_ADDR(func));
        list_add(bps_list, bp);
    }
    else{
        procprint("cannot found function:%s(...)\n", name);
    }
}

void cmd_delete_breakpoint(pid_t pid, struct list_node **bps_list, struct list_node *func_list)
{
    char name[FUNC_NAME_LEN];
    scanf("%s", name);
    struct list_node *func = list_search(func_list, search_func_byname, (void*)name);
    if(func == NULL){
        procprint("cannot found function:%s(...).\n", name);
        return ;
    }
    struct list_node *bp = list_search(*bps_list, is_breakpoint,(void*)FUNC_ADDR(func));
    if(bp == NULL){
        procprint("cannot found breakpoint:%s(...).\n", name);
    }
    ptrace(PTRACE_POKETEXT, pid, (void*)BREAKPOINT_ADDR(bp), (void*)BREAKPOINT_INS(bp));
    list_delete_byfeature(bps_list, is_delete_bp, (void*)FUNC_ADDR(func));
}

void cmd_list_breakpoinst(struct list_node *bps_list)
{
    list_traverse(bps_list, traverse_bps);
}

void cmd_display_ins_eip(pid_t pid)
{
    proc_ins_eip(pid);
}

void cmd_continue(pid_t pid, int *why_stop, struct list_node *bps_list)
{
    struct breakpoint_t *breakpoint;
    if(COME_ACROSS_BREAKPOINT == *why_stop){
        breakpoint = which_breakpoint(pid, bps_list);
        dance_on_breakpoint(pid, breakpoint);
    }
    ptrace(PTRACE_CONT, pid, 0, 0);
    xibugger_wait(pid);
    *why_stop = COME_ACROSS_BREAKPOINT;
}

void cmd_nextline(pid_t pid, int *why_stop, struct list_node *bps_list)
{
    struct breakpoint_t *breakpoint;
    if(COME_ACROSS_BREAKPOINT == *why_stop){
        breakpoint = which_breakpoint(pid, bps_list);
        dance_on_breakpoint(pid, breakpoint);
        *why_stop = !COME_ACROSS_BREAKPOINT;
    }
    else{
        execute_singlestep(pid);
    }
}

void cmd_run(pid_t pid)
{
    ptrace(PTRACE_CONT, pid, 0, 0);
    xibugger_wait(pid);
}

void cmd_help(void)
{
    printf(
        "just like gdb.\n"
        "b -- set breakpoint, e.g. b funcion_name\n"
        "d -- delete breakpoint, e.g. b funcion_name\n"
        "i -- display target's current instruction & eip\n"
        "c -- continue\n"
        "n -- next CPU instruction\n"
        "r -- start the target\n"
        "lbp -- display all breakpoints\n"
        "help(h) -- help infos\n"
    );
}

void run_debugger(pid_t pid, struct list_node *func_list)
{
    //  wait target stoped at 1st instruction
    int status;
    wait(&status);

    int  why_stop = COME_ACROSS_BREAKPOINT;//  come across breakpoint
                                           // or single step (such as after pressing `n`)

    struct list_node *bps_list = NULL; // breakpoints list
    char cmd[XIBUGGER_CMD_LEN]="";
    int  is_running = 0; // is the program/target/tracee running
    while(1){
        procprint("");
        if( EOF == scanf("%s", cmd) ){
            procprint("done\n");
            break;
        }
        if( !strncmp(cmd, "b", XIBUGGER_CMD_LEN) ){ // set breakpoint
            cmd_set_breakpoint(pid, &bps_list, func_list);
            eatendline();
            continue; // get next cmd
        }
        else if( !strncmp(cmd, "d", XIBUGGER_CMD_LEN) ){ // delete a breakpoint
            cmd_delete_breakpoint(pid, &bps_list, func_list);
            why_stop = !COME_ACROSS_BREAKPOINT;
            eatendline();
            continue; // get next cmd
        }
        else if( !strncmp(cmd, "lbp", XIBUGGER_CMD_LEN) ){ // list breakpoints
            cmd_list_breakpoinst(bps_list);
        }
        else if( !strncmp(cmd, "i", XIBUGGER_CMD_LEN) ){ // print target's current instruction & eip
            cmd_display_ins_eip(pid);
        }
        else if( !strncmp(cmd, "c", XIBUGGER_CMD_LEN) ){ // continue
            if( is_running != 1 ){
                procprint("The program is not being run.\n");
                eatendline();
                continue;
            }
            cmd_continue(pid, &why_stop, bps_list);
            prompt_break(pid, bps_list, func_list);
        }
        else if( !strncmp(cmd, "n", XIBUGGER_CMD_LEN) ){ // next line
            // In fact, the cmd `n' in here meas next CPU instruction intread of C code.
            // In general, a line of C code corresponding to a mutilline CPU instruction.
            if( is_running != 1 ){
                procprint("The program is not being run.\n");
                eatendline();
                continue;
            }
            cmd_nextline(pid, &why_stop, bps_list);
        }
        else if( !strncmp(cmd, "r", XIBUGGER_CMD_LEN) ){ // start to run the target
            if(is_running == 1){
                procprint("target is being run\n");
                eatendline();
                continue;
            }
            is_running = 1;
            cmd_run(pid);
            prompt_break(pid, bps_list, func_list);
        }
        else if( !strncmp(cmd, "h", XIBUGGER_CMD_LEN) || !strncmp(cmd, "help", XIBUGGER_CMD_LEN) ){
            cmd_help();
        }
        else {
            procprint("no support for this operation\n");
            eatendline();
            continue;
        }
        eatendline();

    }
    list_destroy(&bps_list);
    list_destroy(&func_list);
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
    struct func_info_t *pdata = NULL;
    struct list_node *h=NULL;
    while( EOF != fscanf(fp, "%s%x", fun_name, &addr) ){
        pdata = (struct func_info_t*)malloc(sizeof(struct func_info_t));
        assert(pdata);
        strncpy(pdata->name, fun_name, FUNC_NAME_LEN);
        pdata->addr = addr;
        list_add(&h, pdata);
    }

    printf("===hints===\n");
    struct list_node *s=h;
    while(s){
        printf("%s : %x\n",
               ((struct func_info_t*)s->pdata)->name,
               ((struct func_info_t*)s->pdata)->addr);
        s = s->next;
    }
    printf("===hints===\n");

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
        /*
         * dwarf() search target's (function-name : address), save them in a file
         * example:
         *  main 0x80486b4
         *  foo  0x8048623
         *  bar  0x8048233
         *  ...
         */
        if( dwarf(2, dwarf_argv) != 0 ){
            perror("cannot read debug infos.\ncompile with -g option?\n");
            return -1;
        }

        // read file for getting all (function-name : address), save them in a link list
        struct list_node *func_list = func_addr(argv[1]);
        assert(func_list);
        run_debugger(pid, func_list);
    }
    else{
        perror("error fork()\n");
    }

    return 0;
}

// ----------------------------------------------------
// used for list operation

int search_func_byname(struct list_node *s, void *data)
{
    return !strncmp( ((struct func_info_t *)s->pdata)->name,
            ((struct func_info_t *)data)->name,
            FUNC_NAME_LEN ) ;
}

int search_func_byaddr(struct list_node *s, void *data)
{
    return  (FUNC_ADDR(s) == (int)data);
}

/*
 * address is the unique identifier of a breakpoint.
 */
int search_bps(struct list_node *node, void *eip)
{
    return (BREAKPOINT_ADDR(node) == (int)eip);
}

int traverse_bps(struct list_node *bp)
{
    printf("%x\n", BREAKPOINT_ADDR(bp));
    return 0;
}

#undef COME_ACROSS_BREAKPOINT
#undef XIBUGGER_CMD_LEN
#undef FUNC_ADD
#undef BREAKPOINT_ADD
#undef BREAKPOINT_IN
#undef FUNC_NAME_LEN
