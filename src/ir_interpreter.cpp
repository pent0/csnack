#include <snack/ir_interpreter.h>
#include <snack/ir_opcode.h>
#include <snack/unit_manager.h>

namespace snack::ir::backend {
    void ir_interpreter::pop(ir_interpreter_func_context &context) {
        context.pop();
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

        context.push(element);
    }

    void ir_interpreter::ldarg(ir_interpreter_func_context &context) {
        context.pc += 2;
        char idx = *(context.ir_bin + context.pc);

        if (idx >= context.local_args.size()) {
            // throw error to error manager
            return;
        }

        context.pc += 1;

        context.push(context.local_args[idx]);
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
        context.push(std::move(element));
    }

    void ir_interpreter::ldlc(ir_interpreter_func_context &context) {
        context.pc += 2;
        char idx = *(context.ir_bin + context.pc);

        if (idx >= context.local_slots.size()) {
            // throw error to error manager
            return;
        }

        context.pc += 1;

        context.push(context.local_slots[idx]);
    }

    void ir_interpreter::add(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.str_data = el2.str_data + el1.str_data;

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el1.num_data + el2.num_data;

        context.push(el_ret);
    }

    void ir_interpreter::sub(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el2.num_data - el1.num_data;

        context.push(el_ret);
    }

    void ir_interpreter::mul(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el1.num_data * el2.num_data;

        context.push(el_ret);
    }

    void ir_interpreter::div(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = el2.num_data / el1.num_data;

        context.push(el_ret);
    }

    void ir_interpreter::mod(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data % (int64_t)el1.num_data;

        context.push(el_ret);
    }

    void ir_interpreter::shl(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data << (int64_t)(el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::shr(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (int64_t)el2.num_data >> (int64_t)(el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::pwr(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            // throw error
            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = std::pow(el2.num_data, el1.num_data);

        context.push(el_ret);
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
        
    }

    void ir_interpreter::strarg(ir_interpreter_func_context &context) {
        context.pc += 2;
        const char idx = *reinterpret_cast<const char *>(context.ir_bin + context.pc);

        if (idx >= context.local_args.size()) {
            // throw error
            return;
        }

        context.pc += 1;

        context.local_args[idx] = std::move(context.evaluation_stack.top());
        
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

    void ir_interpreter::ceq(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.num_data = (el2.str_data == el1.str_data);

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (el2.num_data == el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::cgt(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.num_data = (el2.str_data > el1.str_data);

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (el2.num_data > el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::cge(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.num_data = (el2.str_data >= el1.str_data);

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (el2.num_data >= el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::clt(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.num_data = (el2.str_data < el1.str_data);

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (el2.num_data < el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::cle(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            ir_element el_ret;
            el_ret.type = ir_element::str;
            el_ret.num_data = (el2.str_data <= el1.str_data);

            context.push(el_ret);

            return;
        }

        ir_element el_ret;
        el_ret.type = ir_element::num;
        el_ret.num_data = (el2.num_data <= el1.num_data);

        context.push(el_ret);
    }

    void ir_interpreter::ble(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            if (el2.str_data <= el1.str_data) {
                context.pc = jump_pc;
            }

            return;
        }

        if (el2.num_data <= el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::blt(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            if (el2.str_data < el1.str_data) {
                context.pc = jump_pc;
            }

            return;
        }

        if (el2.num_data < el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::beq(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            if (el2.str_data == el1.str_data) {
                context.pc = jump_pc;
            }

            return;
        }

        if (el2.num_data == el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::bge(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();

        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            if (el2.str_data >= el1.str_data) {
                context.pc = jump_pc;
            }

            return;
        }

        if (el2.num_data >= el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::bgt(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        ir_element el2 = context.pop();
        
        if (el1.type == ir_element::str || el2.type == ir_element::str) {
            if (el2.str_data > el1.str_data) {
                context.pc = jump_pc;
            }

            return;
        }

        if (el2.num_data > el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::br(ir_interpreter_func_context &context) {
        context.pc += 2;
        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc = jump_pc;
    }

    void ir_interpreter::brt(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();

        if (el1.type == ir_element::num && el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::brf(ir_interpreter_func_context &context) {
        context.pc += 2;

        const size_t jump_pc = *reinterpret_cast<const size_t *>(context.ir_bin + context.pc);

        context.pc += sizeof(size_t);

        ir_element el1 = context.pop();
        
        if (el1.type == ir_element::num && !el1.num_data) {
            context.pc = jump_pc;
        }
    }

    void ir_interpreter::uno(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();

        if (el1.type != ir_element::str) {
            el1.num_data = !(el1.num_data);
            context.push(el1);
        } else {
            // report
        }
    }

    void ir_interpreter::ung(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();

        if (el1.type != ir_element::str) {
            el1.num_data = -(el1.num_data);
            context.push(el1);
        } else {
            // report
        }
    }

    void ir_interpreter::uin(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element el1 = context.pop();

        if (el1.type != ir_element::str) {
            el1.num_data = ~(int64_t)(el1.num_data);
            context.push(el1);
        } else {
            // report
        }
    }

    void ir_interpreter::newarr(ir_interpreter_func_context &context) {
        context.pc += 2;

        uint64_t arr = ref_manager.make_new_array();

        ir_element el;
        el.type = ir_element::ref;
        el.ref_id = arr;

        context.push(std::move(el));
    }

    void ir_interpreter::strelm(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element val = context.pop();
        ir_element index = context.pop();
        ir_element arr_ref = context.pop();
        

        if (arr_ref.type != ir_element::ref) {
            // report
            return;
        }

        ir_object_base_ptr obj = ref_manager.get_obj(arr_ref.ref_id);

        if (!obj) {
            // report
            return;
        }

        ir_element *el_arr;

        switch (obj->get_type()) {
        case ir_object_base_type::array: {
            if (index.type != ir_element::num) {
                // report
                return;
            }

            el_arr = &(std::dynamic_pointer_cast<ir_array>(obj))->get_element(index.num_data);
            break;
        }

        default: {
            // Operator overload, call functions

            //report
            return;
        }
        }

        if (el_arr->type == ir_element::none) {
            el_arr->type = val.type;
            el_arr->num_data = val.num_data;
            el_arr->str_data = val.str_data;
        } else if (el_arr->type == ir_element::str) {
            el_arr->str_data = val.str_data;
        } else if (el_arr->type == ir_element::num) {
            el_arr->num_data = val.num_data;
        }
    }

    void ir_interpreter::ldelm(ir_interpreter_func_context &context) {
        context.pc += 2;

        ir_element index = context.pop();
        ir_element arr_ref = context.pop();

        if (arr_ref.type != ir_element::ref) {
            // report
            return;
        }

        ir_object_base_ptr obj = ref_manager.get_obj(arr_ref.ref_id);

        if (!obj) {
            // report
            return;
        }

        ir_element *el_arr;

        switch (obj->get_type()) {
        case ir_object_base_type::array: {
            if (index.type != ir_element::num) {
                // report
                return;
            }

            context.push((std::dynamic_pointer_cast<ir_array>(obj))->get_element(index.num_data));

            break;
        }

        default: {
            // Operator overload, call functions

            //report
            return;
        }
        }
    }

    void ir_interpreter::ret(ir_interpreter_func_context &context) {
        context.pc += 2;

        if (context.evaluation_stack.size() > 1) {
            // throw error
            return;
        }

        if (context.evaluation_stack.size() == 1) {
            ir_element el = context.pop();
            

            func_contexts.pop();
            func_contexts.top().evaluation_stack.push(std::move(el));

            return;
        }

        func_contexts.pop();
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
            { ir::opcode::endmet, BRIDGE(endmet) },
            { ir::opcode::ceq, BRIDGE(ceq) },
            { ir::opcode::clt, BRIDGE(clt) },
            { ir::opcode::cle, BRIDGE(cle) },
            { ir::opcode::cgt, BRIDGE(cgt) },
            { ir::opcode::cge, BRIDGE(cge) },
            { ir::opcode::beq, BRIDGE(beq) },
            { ir::opcode::bge, BRIDGE(bge) },
            { ir::opcode::bgt, BRIDGE(bgt) },
            { ir::opcode::ble, BRIDGE(ble) },
            { ir::opcode::blt, BRIDGE(blt) },
            { ir::opcode::br, BRIDGE(br) },
            { ir::opcode::brt, BRIDGE(brt) },
            { ir::opcode::brf, BRIDGE(brf) },
            { ir::opcode::uno, BRIDGE(uno) },
            { ir::opcode::uin, BRIDGE(uin) },
            { ir::opcode::ung, BRIDGE(ung) },
            { ir::opcode::newarr, BRIDGE(newarr) },
            { ir::opcode::strelm, BRIDGE(strelm) },
            { ir::opcode::ldelm, BRIDGE(ldelm) },
            { ir::opcode::ret, BRIDGE(ret) },
            { ir::opcode::ldarg, BRIDGE(ldarg) },
            { ir::opcode::strarg, BRIDGE(strarg) }
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

    ir_object_base::ir_object_base(const ir_object_base_type type)
        : type(type)
        , id(0)
        , ref_counter(1) {
    }

    ir_object_base::ir_object_base()
        : id(0)
        , ref_counter(1) {
    }

    ir_oop_object::ir_oop_object()
        : ir_object_base(ir_object_base_type::oop) {
    }

    ir_array::ir_array()
        : ir_object_base(ir_object_base_type::array) {
    }

    ir_element &ir_array::get_element(int idx) {
        if (elements.size() < idx + 1) {
            elements.resize(idx + 1);
        }

        return elements[idx];
    }

    size_t ir_array::get_array_length() {
        return elements.size();
    }

    ir_interpreter_ref_manager::ir_interpreter_ref_manager()
        : ref_id(0) {
    }

    uint64_t ir_interpreter_ref_manager::new_id() const {
        ref_id++;
        return ref_id.load();
    }

    uint64_t ir_interpreter_ref_manager::make_new_array() {
        const uint64_t id = new_id();
        objects.emplace(id, std::move(std::make_shared<ir_array>()));

        return id;
    }

    ir_object_base_ptr ir_interpreter_ref_manager::get_obj(uint64_t id) {
        if (objects.find(id) != objects.end()) {
            return objects[id];
        }

        return nullptr;
    }

    void ir_interpreter_func_context::push(ir_element &el) {
        evaluation_stack.push(std::move(el));

        if (el.type == ir_element::ref) {
            ref_manager->do_push(el.ref_id);
        }
    }

    void ir_interpreter_func_context::push(long double val) {
        ir_element el(val);
        push(el);
    }

    void ir_interpreter_func_context::push(const std::string &val) {
        ir_element el(val);
        push(el);
    }

    ir_element ir_interpreter_func_context::pop() {
        ir_element el = std::move(evaluation_stack.top());
        evaluation_stack.pop();

        if (el.type == ir_element::ref) {
            ref_manager->do_pop(el.ref_id);
        }

        return el;
    }

    void ir_interpreter_ref_manager::do_push(uint64_t ref) {
        if (objects.find(ref) != objects.end()) {
            objects[ref]->ref_counter += 1;
        }
    }

    void ir_interpreter_ref_manager::do_pop(uint64_t ref) {
        if (objects.find(ref) != objects.end()) {
            objects[ref]->ref_counter -= 1;

            if (objects[ref]->ref_counter == 0) {
                objects.erase(ref);
            }
        }
    }
}