#pragma once

#include <cstdint>
#include <unordered_map>

namespace snack::ir {
    enum class opcode: uint16_t {
        #define IR_OP_DEF(a) a ,
        #include <snack/ir_opcode.def>
        #undef IR_OP_DEF
        total_opcode
    };

    const char *get_op_name(opcode op);
}

namespace std {
    template <>
    struct hash<snack::ir::opcode> {
        std::size_t operator()(const snack::ir::opcode &c) const {
            return static_cast<std::size_t>(c);
        }
    };
}