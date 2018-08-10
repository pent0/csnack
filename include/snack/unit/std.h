#pragma once

#include <snack/unit.h>

namespace snack::userspace {
    class std_unit : public external_unit {
        void print(ir::backend::ir_interpreter_func_context &context);
        void sin(ir::backend::ir_interpreter_func_context &context);
        void cos(ir::backend::ir_interpreter_func_context &context);
        void tan(ir::backend::ir_interpreter_func_context &context);

        void fopen(ir::backend::ir_interpreter_func_context &context);
        void fclose(ir::backend::ir_interpreter_func_context &context);

    public :
        explicit std_unit();
    };
}