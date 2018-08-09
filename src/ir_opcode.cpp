#include <snack/ir_opcode.h>

namespace snack::ir {
    const char *get_op_name(opcode op) {
        switch (op) {
            #define IR_OP_DEF(a)  \
                case opcode::##a: \
                    return #a;
            #include <snack/ir_opcode.def>
            #undef IR_OP_DEF

         default:
            return nullptr;
        }

        return nullptr;
    }
}