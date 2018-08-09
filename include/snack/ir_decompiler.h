#pragma once

#include <snack/error.h>
#include <snack/ir_compiler.h>

#include <sstream>

namespace snack::ir::frontend {
    class ir_decompiler {
        std::istringstream ir_bin;
        size_t pc = 0;

        backend::ir_binary_header header;
        snack::error_manager *err_mngr;

    protected:
        bool dump_step();

    public:
        explicit ir_decompiler() {}
        ir_decompiler(snack::error_manager &mngr);

        void supply(std::istringstream &stream);

        void dump();
        void dump_unit_ref_table();
    };
}