## Snack compiler

- CSnack is a compiler, decompiler and interpreter for the Snack scripting language. It translates Snack script into SIR 
(Snack Immediate Representation) and interpret them.

- CSnack contains these components:
    - A simple lexer, very fast, using regex expressions.
    - A fast parser.
    - Error manager, manages all the error and dump them when needed.
    - Compiler, compile AST into SIRs
    - Decompiler, which takes SIR binary and decompile them to SIRs
    - Interpreter, which interprets SIR binary until the call stack is wiped out.