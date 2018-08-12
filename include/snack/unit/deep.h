#pragma once

#include <snack/unit.h>

namespace snack::userspace {
    class deep_unit : public external_unit {
        void get_current_method_name(ir::backend::ir_interpreter_func_context &context);
        void get_field_name(ir::backend::ir_interpreter_func_context &context);

    public:
        explicit deep_unit();
    };
}