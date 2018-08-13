## Snack

- Snack is a simple, non object-orientied scripting language, with fast lexing and parsing language, design
for scripting emulator events.
- Some emulator events are scripted. For example: some panic are extracted with script. Recompiling 
custom panic extraction in C++ code takes really long times considering how roundy libraries are depend
on each other, so scripting is a fun way to not recompiling but still provide fast speed.
- Snack is an extra dependency. User can write custom frontend (jit, etc..) to run the AST tree, and write custom
unit (library) for the language

## CSnack
- This repo contains source code for CSnack, Snack compiler, decompiler and interpreter. Snack and CSnack is written in 5 days, 
and this is the result.
- Please traverse to **specs** to read more about the design of CSnack and Snack.

## What is working now, and what to do
- Basic stuffs are done. Loading a host handcoded unit is supported, calling function and do basic variable allocation.
- Arrays, loops, conditional statement are implemented. Array is reference object, design is currently allowed for class and struct.
- There is no struct defintion yet, calling *std.* is prohibited.

## Example script: 
   ```python
   uses std
   
   fn max(a, b):
        if a > b:
           ret a
    
        ret b
    
   fn test:
       var a = 6
       var b = 7 + max(5, 6)
       var c = new array(12, 15, 27)
       for var i = 0; i < length(c); i+=1:
           print('{} ', c[i])
           
   fn main:
       test()
   ```
   
## Analys result
  ```
  IR emitted:

  met 2 0x171
  ldarg 0x0
  ldarg 0x1
  cgt
  brf 0x23
  ldarg 0x0
  ret
  ldarg 0x1
  ret
  endmet

  met 0 0x17e
  ldcst 0x1c5
  strlc 0x0
  ldcst 0x1d9
  ldcst 0x1c5
  ldcst 0x1cf
  call 7fff 0
  add
  strlc 0x1
  newarr
  strlc 0x2
  ldlc 0x2
  ldcst 0x1a7
  ldcst 0x1bb
  strelm
  ldlc 0x2
  ldcst 0x1e3
  ldcst 0x1ed
  strelm
  ldlc 0x2
  ldcst 0x1b1
  ldcst 0x1f7
  strelm
  ldlc 0x2
  strlc 0x2
  ldcst 0x1a7
  strlc 0x3
  ldlc 0x3
  ldlc 0x2
  call 0 4
  clt
  brf 0xf1
  ldlc 0x2
  ldlc 0x3
  ldelm
  ldcststr 0x18c
  call 0 0
  ldlc 0x3
  ldcst 0x1e3
  add
  strlc 0x3
  br 0xa5
  endmet

  met 0 0x199
  call 7fff 1
  endmet

  strdata max
  strdata test
  strdata {}
  strdata main
  idata 0
  idata 2
  idata 12
  idata 6
  idata 5
  idata 7
  idata 1
  idata 15
  idata 27

  Used unit
  std

  Interpreted result:
  12 15 27
  ```
