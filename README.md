# xibugger

### a simple debugger, a toy, a demo.

## environment
written for 32 bit Linux(x86). These code is closely related to OS and architecture.

## reference
xibugger is originally written based on code from [it](http://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/) by Eli Bendersky under the public domain. Most if not all of the code has been rewritten. xibugger is licensed under "GPL v2" license.

## how to run
`make`

`make run`

**note** support only *one* breakpoint now. and have to enter breakpoint-address instread of line-number or 
function name. 

use `objdump -d your-target` get the address of breakpoint you want.

### example

`make`

`objdump -d traced | grep catch_me`, note the address of function catch\_me().

`make run`, enter the address you note. and press ENTER to continue.


## todo

multiple breakpoint

delete breakpoint

break with line-number and function name

`n` => nextstep

`b xxx` => set breakpoint

`c` => continue

cmd readline library support
