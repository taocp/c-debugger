/*
 * =====================================================================================
 *
 *       Filename:  dwarf.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/29/2014 08:40:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef XIBUGGER_DWARF_H_INCLUDE
#define XIBUGGER_DWARF_H_INCLUDE

// XXX there must be a standard rules for maximum file name length.
#define TARGET_NAME_LEN 50

// the suffix of file note the function name and it's address
// example:
// if suffix == ".func_addr"
// then file "a.out.func_addr" note infos for executable file "a.out"
extern const char *suffix;

int dwarf(int argc, char** argv);

#endif
