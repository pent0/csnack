#include <snack/ir_interpreter.h>
#include <snack/ir_opcode.h>
#include <snack/unit_manager.h>

namespace snack::ir::backend {
    void ir_interpreter::pop(ir_interpreter_func_context &context) {
        context.evaluation_stack.pop();
        context.pc += 2;
    }

    void ir_interpreter::ldcst(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t data_addr = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);
        const opcode op = *reinterpret_cast<const opcode *>(context.ir_global_bin + data_addr);

        if (op != opcode::idata) {
            // report
            return;
        }

        ir_element element;
        element.type = ir_element::num;
        element.num_data = *reinterpret_cast<const long double *>(context.ir_global_bin + data_addr + 2);

        context.pc += 8;

        context.evaluation_stack.push(std::move(element));
    }

    void ir_interpreter::ldcststr(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t data_addr = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);
        const opcode op = *reinterpret_cast<const opcode *>(context.ir_global_bin + data_addr);

        if (op != opcode::strdata) {
            // report
            return;
        }

        ir_element element;
        element.type = ir_element::str;

        char *str = const_cast<char *>(context.ir_global_bin) + data_addr + 2;
        size_t len = *reinterpret_cast<size_t *>(str);

        element.str_data.resize(len);
        str += sizeof(size_t);

        std::copy(str, str + len, element.str_data.begin());

        context.pc += 8;
        context.evaluation_stack.push(std::move(element));
    }

    void ir_interpreter::ldlc(ir_interpreter_func_context &context) {
        context.pc += 2;
        char idx = *(context.ir_bin + context.pc);

        if (idx >= context.local_slots.size()) {
            // throw error to error manager
            return;
        }

        context.pc += 1;

        context.evaluation_stack.push(context.local_slots[idx]);
    }

    void ir_interpreter::add(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.str_data = el2.str_data + el1.str_data;

            context.evaluation_stack.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el1.num_data + el2.num_data;

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::sub(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el2.num_data - el1.num_data;

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::mul(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el1.num_data * el2.num_data;

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::div(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el2.num_data / el1.num_data;

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::mod(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data % (int64_t)el1.num_data;

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::shl(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data << (int64_t)(el1.num_data);

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::shr(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data >> (int64_t)(el1.num_data);

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::pwr(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.evaluation_stack.top();
        context.evaluation_stack.pop();
        ir_element el2 = context.evaluation_stack.top();
        context.evaluation_stack.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = std::pow(el2.num_data, el1.num_data);

        context.evaluation_stack.push(el_ret);
    }

    void ir_interpreter::met(ir_interpreter_func_context &context) {
        context.pc += 2;
        const uint16_t arg_count = *reinterpret_cast<const uint16_t *>(context.ir_bin + context.pc);

        if (arg_count && last_context) {
            for (size_t i = 0; i < arg_count; i++) {
                ir_element el = std::move(last_context->evaluation_stack.top());
                last_context->evaluation_stack.pop();
                context.local_args[i] = std::move(el);
            }
        }

        context.pc += 10;
    }

    void ir_interpreter::strlc(ir_interpreter_func_context &context) {
        context.pc += 2;
        const char idx = *reinterpret_cast<const char *>(context.ir_bin + context.pc);

        if (idx >= context.local_slots.size()) {
            // throw error
            return;
        }

        context.pc += 1;

        context.local_slots[idx] = std::move(context.evaluation_stack.top());
        context.evaluation_stack.pop();
    }

    void ir_interpreter::vri(ir_interpreter_func_context &context) {
        context.pc += 2;
        const char idx = *reinterpret_cast<const char *>(context.ir_bin + context.pc);

        if (idx >= context.local_slots.size()) {
            // throw error
            return;
        }

        context.local_args[idx].type = ir_element::num;
        context.local_args[idx].num_data = 0;
    }

    void ir_interpreter::vrs(ir_interpreter_func_context &context) {
        context.pc += 2;
        const char idx = *reinterpret_cast<const char *>(context.ir_bin + context.pc);

        if (idx >= context.local_slots.size()) {
            // throw error
            return;
        }

        context.local_args[idx].type = ir_element::str;
        context.local_args[idx].str_data = "";
    }

    void ir_interpreter::call(ir_interpreter_func_context &context) {
        context.pc += 2;

        uint16_t unit_jump = *reinterpret_cast<const uint16_t *>(context.ir_bin + context.pc);
        uint16_t func_jump = *reinterpret_cast<const uint16_t *>(context.ir_bin + context.pc + 2);

        context.pc += 4;
        last_context = &context;

        if (unit_jump == 0x7FFF) {
            bool res = context.owning_unit->call_function(this, func_jump, &context);

            if (!res) {
                // do report
                return;
            }
        } else {
            auto unit_name = context.owning_unit->get_reference_unit_name(unit_jump);

            if (!unit_name) {
                // do report
                return;
            }

            userspace::unit_ptr call_unit = unit_mngr->use_unit(*unit_name);

            if (!call_unit) {
                // do report
                return;
            }

            bool res = call_unit->call_function(this, func_jump, &context);

            if (!res) {
                // do report
                return;
            }
        }
    }

    ir_interpreter_func_context &ir_interpreter::get_current_func_context() {
        return func_contexts.top();
    }

    void ir_interpreter::endmet(ir_interpreter_func_context &context) {
        context.pc += 2;

        func_contexts.pop();
    }

#define BRIDGE(a) std::bind(&ir_interpreter::##a, this, std::placeholders::_1)

    void ir_interpreter::init_opcode_table() {
        opcode_map = {
            { ir::opcode::add, BRIDGE(add) },
            { ir::opcode::sub, BRIDGE(sub) },
            { ir::opcode::mul, BRIDGE(mul) },
            { ir::opcode::div, BRIDGE(div) },
            { ir::opcode::mod, BRIDGE(mod) },
            { ir::opcode::shl, BRIDGE(shl) },
            { ir::opcode::shr, BRIDGE(shr) },
            { ir::opcode::pwr, BRIDGE(pwr) },
            { ir::opcode::ldcst, BRIDGE(ldcst) },
            { ir::opcode::ldcststr, BRIDGE(ldcststr) },
            { ir::opcode::ldlc, BRIDGE(ldlc) },
            { ir::opcode::met, BRIDGE(met) },
            { ir::opcode::strlc, BRIDGE(strlc) },
            { ir::opcode::vri, BRIDGE(vri) },
            { ir::opcode::vrs, BRIDGE(vrs) },
            { ir::opcode::call, BRIDGE(call) },
            { ir::opcode::endmet, BRIDGE(endmet) }
        };
    }

    void ir_interpreter::do_opcode() {
        ir_interpreter_func_context &context = get_current_func_context();
        const ir::opcode op = *reinterpret_cast<const ir::opcode *>(context.ir_bin + context.pc);

        opcode_map[op](context);
    }

    ir_interpreter::ir_interpreter(snack::error_manager &err_mngr, snack::userspace::unit_manager &manager)
        : err_manager(&err_mngr)
        , unit_mngr(&manager) {
        init_opcode_table();
    }

    void ir_interpreter::interpret() {
        while (!func_contexts.empty()) {
            do_opcode();
        }
    }
}