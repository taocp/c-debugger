To explain some strange code.

### cmd `n`

not support next line C code, just support next line CPU instruction.

so, `n` in this debugger means next CPU instruction intead of C code.

---

###  #define COME_ACROSS_BREAKPOINT 0

original instructions:
```
1:0x1256
2:0x8967
3:0xabcd
```
after  setting breakpoint on 1:
```
1:0xcc56
2:0x8967
3:0xabcd
```

now, target has stopped at 1

**if we enter `n` now**, the debugger should
- change `0xcc56` into `0x1256`,
- excute `0x1256`,
- change `0xcc12` into `0xcc56` again.

now, eip point to `0x8967`.

**if we enter `n` now**, the debugger should
- just excute `0x8967`, without dancing on breakpoint.

so, it's necessary to note why stopped.

---
