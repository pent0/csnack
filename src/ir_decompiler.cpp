#include <snack/ir_compiler.h>
#include <snack/ir_decompiler.h>
#include <snack/ir_opcode.h>

#include <iostream>

namespace snack::ir::frontend {
    ir_decompiler::ir_decompiler(snack::error_manager &mngr)
        : err_mngr(&mngr) {
    }

    void ir_decompiler::supply(std::istringstream &stream) {
        ir_bin = std::move(stream);

        ir_bin.read(reinterpret_cast<char *>(&header), sizeof(header));

        pc = header.code_addr;
        ir_bin.seekg(pc);
    }

    void ir_decompiler::dump() {
        while (dump_step()) {
        }
    }

    bool ir_decompiler::dump_step() {
        if ((ir_bin.eof() || ir_bin.fail()) || ir_bin.tellg() >= header.relocate_addr) {
            return false;
        }

        opcode op;
        ir_bin.read(reinterpret_cast<char *>(&op), 2);

        pc += 2;

        const char *name = get_op_name(op);

        if (!name) {
            return false;
        }

        std::cout << name;

        switch (op) {
        case opcode::ldcststr:
        case opcode::ldcst:
        case opcode::beq:
        case opcode::bge:
        case opcode::bgt:
        case opcode::blt:
        case opcode::ble: 
        case opcode::br: 
        case opcode::brt: 
        case opcode::brf: {
            size_t off = 0;
            ir_bin.read(reinterpret_cast<char *>(&off), sizeof(size_t));

            std::cout << " 0x" << std::hex << off;

            pc += sizeof(size_t);

            break;
        }

        case opcode::met: {
            uint16_t total_arg = 0;
            ir_bin.read(reinterpret_cast<char *>(&total_arg), 2);

            std::cout << " " << total_arg;

            size_t off = 0;
            ir_bin.read(reinterpret_cast<char *>(&off), sizeof(size_t));

            std::cout << " 0x" << std::hex << off;

            pc += sizeof(size_t) + 2;

            break;
        }

        case opcode::ldarg:
        case opcode::ldlc:
        case opcode::strarg:
        case opcode::strlc:
        case opcode::vri:
        case opcode::vrs: {
            uint8_t idx = 0;
            ir_bin.read(reinterpret_cast<char *>(&idx), 1);

            std::cout << " 0x" << std::hex << (int)idx;

            pc += 1;

            break;
        }

        case opcode::endmet: {
            std::cout << std::endl;
            break;
        }

        case opcode::idata: {
            long double val;
            ir_bin.read(reinterpret_cast<char *>(&val), sizeof(long double));

            std::cout << " " << val;

            pc += sizeof(long double);
            break;
        }

        case opcode::strdata: {
            std::string str;
            size_t str_len;

            ir_bin.read(reinterpret_cast<char *>(&str_len), sizeof(size_t));
            str.resize(str_len);
            ir_bin.read(reinterpret_cast<char *>(&str[0]), str_len);

            std::cout << " " << str;

            pc += sizeof(size_t) + str_len;
            break;
        }

        case opcode::call: {
            int16_t idx;
            ir_bin.read(reinterpret_cast<char *>(&idx), 2);

            std::cout << " " << (int)idx;

            ir_bin.read(reinterpret_cast<char *>(&idx), 2);
            std::cout << " " << (int)idx;

            break;
        }
        }

        std::cout << std::endl;

        return true;
    }

    void ir_decompiler::dump_unit_ref_table() {
        ir_bin.seekg(header.ref_table_addr);

        for (size_t i = 0; i < header.ref_count; i++) {
            size_t str_len = 0;
            ir_bin.read(reinterpret_cast<char *>(&str_len), sizeof(size_t));

            std::string unit_name;
            unit_name.resize(str_len);

            ir_bin.read(reinterpret_cast<char *>(&unit_name[0]), str_len);

            std::cout << unit_name << std::endl;
        }
    }
}