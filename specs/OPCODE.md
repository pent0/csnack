- *call:*
  - Following the opcodes, currently is:
     - The index of unit contains the function in the unit reference table of the current unit (size_t)
     - Reserved for class table reference (size_t)
     - The index of the function in the unit (behavior may change in the future) (size_t)
  - The opcode does:
     - Push a new func context to the function contexts stack.

- *met:*
   - Following the opcode, currently is:
       - Argument count (size_t)
       - Pointer to the method name in binary file (size_t)
       
   - The opcode does:  
       - Popped arguments count from previous function context and store it in local arguments slot
       
- *ret:*
   - Opcode does:
       - Pop the last element from evaluation stack of current thread context
       - Pop the func contexts stack
       - Push the element that just popped to the evaluation stack of current thread context.
