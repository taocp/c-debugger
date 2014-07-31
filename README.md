# xibugger

### a simple C debugger, a toy, a demo.

written for 32 bit Linux. These code is closely related to OS and architecture.

I test it on Gentoo(x86). `gcc version 4.7.3`.

## how to run
`make`

`./xibugger your-target` ( make sure target compiled with `-g` option )

`r`, `n`, `c`, `b` cmd just like using gdb.

**note** `b` support only function name. no support for line number.

## todo
~~multiple breakpoint~~

break with line-number ~~and function name~~

delete breakpoint

cmd readline library support

docs:how this debugger work

## reference
a simple debugger is originally written based on code from [here](http://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/) by Eli Bendersky under the public domain. Most if not all of the code has been rewritten. xibugger is licensed under "GPL v2" license.
