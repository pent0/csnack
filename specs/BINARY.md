# SIR binary

- SIR binary consists of 4 parts:
    - Header, contains code address, unit reference table, etc...
    - Code section, contains SIR opcodes
    - Data section, contains static int and string data
    - Relocate section, contains address to relocate when needed
    - Function entry table, consists all entry address of all function in the current unit
    - Unit reference table, consists of all unit names

### Header:
    - Magic header: Contains 'SIL\0'
    - Compiler Version: 1 byte major, 1 byte minor, 1 byte build
    - Code address (8 bytes)
    - Data address (8 bytes)
    - Relocate address (8 bytes)
    - Total function (8 bytes)
    - Function entries table address (8 bytes)
    - Total refrence unit (8 bytes)
    - Unit reference table address (8 bytes)

### Code section:
    - The code section starts at the code address specified the header, and have the size of distance between data and code address.
    - Each opcode consumes 2 bytes, after opcode may contains additional info, so the total bytes will not represent how many opcode
    there is.

### Data section
    - The data section contains all static data. 
    - Data is either number or string. 
    - Number is stored with an **idata** opcode. After the opcode, there will be a static long double number
    - String is stored with an **strdata** opcode. After the opcode, 8 bytes will represent an ANSI string length, and length bytes
    later is the string data.
    - A opcode requested for an **idata** or **strdata**, the pointer to write address of data to will be recorded and filled when
    data section is constructed.

### Relocate section
    - Relocate section contains all address that contains data address that needed to be relocated if the base address is changed.
    Base address by default is 0. Since script is small, relocation is mostly not needed.

### Function entry table
    - Contains all function entry address. Since each function starts with a met instruction, which contains the total arguments 
    and the function name string data address, it's very easy to query function name.

### Unit reference table
    - Contains a list of ANSI string of unit names. 