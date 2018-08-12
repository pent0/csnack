#include <iostream>
#include <snack/unit/std.h>

// Symbian C++ 3.0 programming
namespace snack::userspace {
    void std_unit::print(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element format = context.pop();

        if (format.type != decltype(format)::str) {
            // interpreter error
            return;
        }

        std::string format_str = format.str_data;
        size_t format_pos = format_str.find("{}");

        while (format_pos != std::string::npos) {
            ir::backend::ir_element dat = context.pop();

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
        ir::backend::ir_element num = context.pop();

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        context.push(std::sin(num.num_data));
    }

    void std_unit::cos(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element num = context.pop();

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        context.push(std::cos(num.num_data));
    }

    void std_unit::tan(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element num = context.pop();

        if (num.type != ir::backend::ir_element::num) {
            // report interpreter
            return;
        }

        context.push(std::tan(num.num_data));
    }

    void std_unit::length(ir::backend::ir_interpreter_func_context &context) {
        ir::backend::ir_element el = context.pop();

        switch (el.type) {
        case ir::backend::ir_element::num:
            context.push(-1);
            break;

        case ir::backend::ir_element::str:
            context.push(el.str_data.length());
            break;

        case ir::backend::ir_element::ref: {
            auto obj = context.ref_manager->get_obj(el.ref_id);
            
            if (obj->get_type() == ir::backend::ir_object_base_type::array) {
                std::shared_ptr<ir::backend::ir_array> arr = std::dynamic_pointer_cast<ir::backend::ir_array>(obj);
                context.push(arr->get_array_length());

                break;
            } else {
                context.push(-1);
            }
        }
        }
    }

    std_unit::std_unit()
        : external_unit("std") {
        REGISTER_UNIT_FUNC(std_unit, "print", print, -1);
        REGISTER_UNIT_FUNC(std_unit, "sin", sin, 1);
        REGISTER_UNIT_FUNC(std_unit, "cos", print, 1);
        REGISTER_UNIT_FUNC(std_unit, "tan", print, 1);
        REGISTER_UNIT_FUNC(std_unit, "length", length, 1);
    }
}