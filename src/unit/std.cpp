#include <iostream>
#include <snack/unit/std.h>

// Symbian C++ 3.0 programming
namespace snack::userspace {
#define POP_STACK(context)          \
    context.evaluation_stack.top(); \
    context.evaluation_stack.pop()

    #define PUSH_STACK(context, target) \
    context.evaluation_stack.push(ir::backend::ir_element(target))

    void std_unit::print(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element format = POP_STACK(context);

        if (format.type != decltype(format)::str) {
            // interpreter error
            return;
        }

        std::string format_str = format.str_data;
        size_t format_pos = format_str.find("{}");

        while (format_pos != std::string::npos) {
            ir::backend::ir_element dat = POP_STACK(context);

            if (dat.type == decltype(dat)::num) {
                format_str.replace(format_str.begin() + format_pos, format_str.begin() + format_pos + 2,
                    (int64_t)(dat.num_data) == dat.num_data ? std::to_string((int64_t)(dat.num_data)) : std::to_string(dat.num_data));
            } else {
                format_str.replace(format_str.begin() + format_pos, format_str.begin() + format_pos + 2,
                    dat.str_data);
            }

            format_pos = format_str.find("{}");
        }

        std::cout << format_str;
    }

    void std_unit::sin(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element num = POP_STACK(context);

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        PUSH_STACK(context, std::sin(num.num_data));
    }

    void std_unit::cos(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element num = POP_STACK(context);

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        PUSH_STACK(context, std::cos(num.num_data));
    }

    void std_unit::tan(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element num = POP_STACK(context);

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        PUSH_STACK(context, std::tan(num.num_data));
    }

    std_unit::std_unit()
        : external_unit("std") {
        REGISTER_UNIT_FUNC(std_unit, "print", print, -1);
        REGISTER_UNIT_FUNC(std_unit, "sin", sin, 1);
        REGISTER_UNIT_FUNC(std_unit, "cos", print, 1);
        REGISTER_UNIT_FUNC(std_unit, "tan", print, 1);
    }
}