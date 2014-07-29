# xibugger

### a simple C debugger, a toy, a demo.

written for 32 bit Linux(x86). These code is closely related to OS and architecture.

## reference
xibugger is originally written based on code from [here](http://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/) by Eli Bendersky under the public domain. Most if not all of the code has been rewritten. xibugger is licensed under "GPL v2" license.

## how to run
`make`

`./xibugger your-target` ( make sure target compiled with `-g` option )

**note** support only *one* breakpoint now. and have to enter breakpoint-address instread of line-number or 
function name. 


### example

`make`

`make run`

this debugger would print `function-name : address`

setting breakpoint : `b address`

when the debugger is being run,

press `r`, `n`, `c` just like using gdb.

## todo
multiple breakpoint

break with line-number and function name

delete breakpoint

cmd readline library support
