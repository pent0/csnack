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
- The work todo now is expand basic operation, like allowing branch operation or loop. This is being worked on.

- ###### ***This project is a part of EKA2L1.***
