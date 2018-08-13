#include <snack/ir_compiler.h>
#include <snack/ir_opcode.h>

namespace snack::ir::backend {
    ir_compiler::ir_compiler(snack::error_manager &mngr, snack::userspace::unit_manager &unit_mngr)
        : err_mngr(&mngr)
        , unit_mngr(&unit_mngr) {
    }

    void ir_compiler::do_report(error_panic_code code, error_level level, node_ptr node) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::compiler, level, code, node->get_column(), node->get_line());
        }
    }

    void ir_compiler::do_report(error_panic_code code, error_level level, node_ptr node, const std::string &arg0) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::compiler, level, code, node->get_column(), node->get_line(), arg0);
        }
    }

    void ir_compiler::do_report(error_panic_code code, error_level level, node_ptr node, const std::string &arg0, const std::string &arg1) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::compiler, level, code, node->get_column(), node->get_line(), arg0, arg1);
        }
    }

    void ir_compiler::emit(opcode op) {
        ir_op_info op_info;
        op_info.op = op;
        op_info.bin_addr = ir_bin.tellp();
        op_info.pc_context_addr = funcs.back().crr_pc;

        ir_bin.write(reinterpret_cast<const char *>(&op), 2);

        funcs.back().crr_pc += 2;
        funcs.back().opcodes.push_back(op_info);
    }

    void ir_compiler::emit(opcode op, long double value) {
        ir_op_info op_info;
        op_info.op = op;
        op_info.bin_addr = ir_bin.tellp();
        op_info.pc_context_addr = funcs.back().crr_pc;

        ir_bin.write(reinterpret_cast<const char *>(&op), 2);

        funcs.back().crr_pc += 2;
        funcs.back().opcodes.push_back(op_info);

        switch (op) {
        case opcode::ldcst: {
            relocate_number_list[value].push_back(op_info.bin_addr + 2);
            funcs.back().crr_pc += sizeof(size_t);

            size_t holder = 0;

            ir_bin.write(reinterpret_cast<const char *>(&holder), sizeof(size_t));

            break;
        }

        case opcode::beq:
        case opcode::bge:
        case opcode::bgt:
        case opcode::blt:
        case opcode::ble:
        case opcode::br:
        case opcode::brt:
        case opcode::brf: {
            size_t jump_addr = static_cast<size_t>(value);

            ir_bin.write(reinterpret_cast<const char *>(&jump_addr), sizeof(size_t));
            funcs.back().crr_pc += sizeof(size_t);

            break;
        }

        case opcode::ldarg:
        case opcode::ldlc:
        case opcode::strarg:
        case opcode::strlc:
        case opcode::vri:
        case opcode::vrs: {
            uint16_t idx = static_cast<uint16_t>(value);

            ir_bin.write(reinterpret_cast<const char *>(&idx), 1);
            funcs.back().crr_pc += 1;
            break;
        }

        case opcode::call: {
            uint32_t idx = static_cast<uint32_t>(value);

            ir_bin.write(reinterpret_cast<const char *>(&idx), 4);
            funcs.back().crr_pc += 4;
            break;
        }

        default:
            break;
        }
    }

    void ir_compiler::emit(opcode op, const std::string &value) {
        ir_op_info op_info;
        op_info.op = op;
        op_info.bin_addr = ir_bin.tellp();
        op_info.pc_context_addr = funcs.back().crr_pc;

        ir_bin.write(reinterpret_cast<const char *>(&op), 2);

        funcs.back().crr_pc += 2;
        funcs.back().opcodes.push_back(op_info);

        switch (op) {
        case opcode::ldcststr: {
            relocate_string_list[value].push_back(op_info.bin_addr + 2);

            size_t holder = 0;
            ir_bin.write(reinterpret_cast<const char *>(&holder), sizeof(size_t));

            funcs.back().crr_pc += sizeof(size_t);
            break;
        }

        default:
            break;
        }
    }

    void ir_compiler::emit(opcode op, long double nval, const std::string &sval) {
        ir_op_info op_info;
        op_info.op = op;
        op_info.bin_addr = ir_bin.tellp();
        op_info.pc_context_addr = funcs.back().crr_pc;

        ir_bin.write(reinterpret_cast<const char *>(&op), 2);

        funcs.back().crr_pc += 2;
        funcs.back().opcodes.push_back(op_info);

        switch (op) {
        case opcode::met: {
            uint16_t num_args = static_cast<uint16_t>(nval);
            ir_bin.write(reinterpret_cast<char *>(&num_args), 2);

            relocate_string_list[sval].push_back(op_info.bin_addr + 4);

            size_t holder = 0;
            ir_bin.write(reinterpret_cast<const char *>(&holder), sizeof(size_t));

            funcs.back().crr_pc += sizeof(size_t) + 2;
            break;
        }

        default:
            break;
        }
    }

    void ir_compiler::build_function(function_node_ptr func) {
        ir_function func_ir;
        func_ir.crr_pc = 0;

        funcs.push_back(func_ir);

        emit(opcode::met, func->get_args().size(), func->get_name());

        for (const auto &node : func->get_childrens()) {
            build_node(func, node);
        }

        emit(opcode::endmet);
    }

    void ir_compiler::build_array_push(function_node_ptr func, std::shared_ptr<array_node> node, uint32_t arr_var_index) {
        for (size_t i = 0; i < node->get_init_elements().size(); i++) {
            emit(opcode::ldlc, arr_var_index);
            emit(opcode::ldcst, i);

            build_push_hs(func, node->get_init_elements()[i]);

            emit(opcode::strelm);
        }
    }

    void ir_compiler::build_new_object(function_node_ptr func, std::shared_ptr<new_object_node> node, uint32_t var_index) {
        switch (node->get_new_object_request()->get_node_type()) {
        case node_type::array: {
            emit(opcode::newarr);
            emit(opcode::strlc, var_index);

            build_array_push(func, std::dynamic_pointer_cast<array_node>(node->get_new_object_request()), var_index);

            emit(opcode::ldlc, var_index);

            break;
        }

        default:
            break;
        }
    }

    void ir_compiler::build_push_hs(function_node_ptr func, node_ptr node) {
        if (node == nullptr) {
            return;
        }

        node_type nt = node->get_node_type();

        switch (nt) {
        case node_type::number: {
            std::shared_ptr<number_node> num = std::dynamic_pointer_cast<number_node>(node);
            emit(opcode::ldcst, num->get_value());

            break;
        }

        case node_type::string: {
            std::shared_ptr<string_node> str = std::dynamic_pointer_cast<string_node>(node);
            emit(opcode::ldcststr, str->get_string());

            break;
        }

        case node_type::var:
        case node_type::array_access: {
            std::shared_ptr<array_access_node> v = std::dynamic_pointer_cast<array_access_node>(node);
            node_type nt = node->get_node_type();

            auto var_idx = std::find(func->get_local_vars().begin(), func->get_local_vars().end(),
                nt == node_type::var ? node : v->get_var());

            size_t idx = 0;
            bool is_arg = false;

            bool found = false;

            for (uint8_t i = 0; i < func->get_args().size(); i++) {
                if ((nt == node_type::var && func->get_args()[i] == node)
                    || (nt == node_type::array_access && func->get_args()[i] == v->get_var())) {
                    idx = i;
                    is_arg = true;
                    found = true;

                    break;
                }
            }

            if (!found) {
                if (var_idx == func->get_local_vars().end()) {
                    break;
                }

                idx = var_idx - func->get_local_vars().begin();
            }

            if (node->get_node_type() == node_type::var) {
                if (is_arg) {
                    emit(opcode::ldarg, idx);
                } else {
                    emit(opcode::ldlc, idx);
                }
            } else {
                if (is_arg) {
                    emit(opcode::ldarg, idx);
                } else {
                    emit(opcode::ldlc, idx);
                }

                build_push_hs(func, v->get_index());
                emit(opcode::ldelm);
            }

            break;
        }

        case node_type::null: {
            emit(opcode::ldnull);
            break;
        }

        case node_type::unary: {
            build_unary(func, std::dynamic_pointer_cast<unary_node>(node));
            break;
        }

        default: {
            build_node(func, node);
            break;
        }
        }
    }

    void ir_compiler::build_caculate(function_node_ptr func, std::shared_ptr<caculate_node> node) {
        build_push_hs(func, node->get_lhs());
        build_push_hs(func, node->get_rhs());

        switch (node->get_op()) {
        case caculate_op::add:
            emit(opcode::add);
            break;

        case caculate_op::sub:
            emit(opcode::sub);
            break;

        case caculate_op::div:
            emit(opcode::div);
            break;

        case caculate_op::mul:
            emit(opcode::mul);
            break;

        case caculate_op::and:
        case caculate_op::logical_and:
            emit(opcode::and);
            break;

        case caculate_op:: or:
        case caculate_op::logical_or:
            emit(opcode:: or);
            break;

        case caculate_op:: xor:
            emit(opcode:: xor);
            break;

        case caculate_op::power:
            emit(opcode::pwr);
            break;

        case caculate_op::shl:
            emit(opcode::shl);
            break;

        case caculate_op::shr:
            emit(opcode::shr);
            break;

        case caculate_op::equal:
            emit(opcode::ceq);
            break;

        case caculate_op::greater:
            emit(opcode::cgt);
            break;

        case caculate_op::greater_equal:
            emit(opcode::cge);
            break;

        case caculate_op::less:
            emit(opcode::clt);
            break;

        case caculate_op::less_equal:
            emit(opcode::cle);
            break;

        default:
            break;
        }
    }

    void ir_compiler::build_assign(function_node_ptr func, std::shared_ptr<assign_node> node) {
        node_type ltr = node->get_lhs()->get_node_type();

        switch (ltr) {
        case node_type::var:
        case node_type::array_access: {
            std::shared_ptr<array_access_node> v = std::dynamic_pointer_cast<array_access_node>(node->get_lhs());

            auto var_idx = std::find(func->get_local_vars().begin(), func->get_local_vars().end(),
                ltr == node_type::var ? node->get_lhs() : v->get_var());

            size_t idx = 0;
            bool is_arg = false;
            bool found = false;

            node_type nt = node->get_node_type();

            for (uint8_t i = 0; i < func->get_args().size(); i++) {
                if ((nt == node_type::var && func->get_args()[i] == node->get_lhs())
                    || (nt == node_type::array_access && func->get_args()[i] == v->get_var())) {
                    idx = i;
                    is_arg = true;
                    found = true;

                    break;
                }
            }

            if (!found) {
                if (var_idx == func->get_local_vars().end()) {
                    break;
                }

                idx = var_idx - func->get_local_vars().begin();
            }

            if (ltr == node_type::array_access) {
                if (!is_arg)
                    emit(opcode::ldlc, idx);
                else
                    emit(opcode::ldarg, idx);

                std::shared_ptr<array_access_node> v = std::dynamic_pointer_cast<array_access_node>(node->get_lhs());

                build_push_hs(func, v->get_index());
            }

            switch (node->get_rhs()->get_node_type()) {
            case node_type::new_object: {
                build_new_object(func, std::dynamic_pointer_cast<new_object_node>(node->get_rhs()), idx);
                break;
            }

            case node_type::array: {
                build_array_push(func, std::dynamic_pointer_cast<array_node>(node->get_rhs()), idx);
                break;
            }

            default: {
                build_push_hs(func, node->get_rhs());
                break;
            }
            }

            if (ltr == node_type::var) {
                if (!is_arg)
                    emit(opcode::strlc, idx);
                else
                    emit(opcode::strarg, idx);
            } else {
                emit(opcode::strelm);
            }

            break;
        }

        default:
            break;
        }
    }

    void ir_compiler::build_function_call(function_node_ptr func, std::shared_ptr<function_call_node> func_call) {
        for (int i = func_call->get_args().size() - 1; i >= 0; i--) {
            build_push_hs(func, func_call->get_args()[i]);
        }

        int16_t index_unit = -1;
        int16_t index_func = -1;

        // 0x7FFF = current module
        size_t func_idx = 0;

        for (size_t i = 0; i < target->get_childrens().size(); i++) {
            if (target->get_childrens()[i]->get_node_type() == node_type::function) {
                function_node_ptr func_callee = std::dynamic_pointer_cast<function_node>(target->get_childrens()[i]);

                if (func_callee->get_name() == func_call->get_function()->get_name() && func_callee->get_args().size() == func_call->get_args().size()) {
                    index_unit = 0x7FFF;
                    index_func = func_idx;

                    break;
                }

                func_idx++;
            }
        }

        bool dup_note = false;

        if (index_unit < 0 || index_func < 0) {
            for (size_t i = 0; i < unit_table.size(); i++) {
                userspace::unit_ptr unit = unit_mngr->use_unit(unit_table[i]);

                if (!unit) {
                    continue;
                }

                auto idx_func_temp = unit->get_function_idx(func_call->get_function()->get_name(), func_call->get_args().size());

                if (!idx_func_temp) {
                    continue;
                }

                if (*idx_func_temp >= 0 && index_func >= 0 && !dup_note) {
                    // log note
                    do_report(error_panic_code::duplicate_func,
                        error_level::note, func_call->get_function(), func_call->get_function()->get_name());

                    dup_note = true;
                }

                index_func = *idx_func_temp;
                index_unit = i;
            }
        }

        if (index_unit < 0 || index_func < 0) {
            // log error
            do_report(error_panic_code::func_not_found, error_level::error, func_call->get_function(),
                func_call->get_function()->get_name());

            return;
        }

        emit(opcode::call, (index_func << 16) | index_unit);
    }

    void ir_compiler::build_node(function_node_ptr func, node_ptr node) {
        switch (node->get_node_type()) {
        case node_type::function: {
            build_function(std::dynamic_pointer_cast<function_node>(node));
            break;
        }

        case node_type::caculate: {
            build_caculate(func, std::dynamic_pointer_cast<caculate_node>(node));
            break;
        }

        case node_type::assign: {
            build_assign(func, std::dynamic_pointer_cast<assign_node>(node));
            break;
        }

        case node_type::unit_ref: {
            std::shared_ptr<unit_reference_node> unit_ref = std::dynamic_pointer_cast<unit_reference_node>(node);
            unit_table.push_back(unit_ref->get_unit_ref_name());

            break;
        }

        case node_type::function_call: {
            build_function_call(func, std::dynamic_pointer_cast<function_call_node>(node));
            break;
        }

        case node_type::if_else: {
            build_if_else(func, std::dynamic_pointer_cast<if_else_node>(node));
            break;
        }

        case node_type::block: {
            std::shared_ptr<block_node> block = std::dynamic_pointer_cast<block_node>(node);

            for (const auto &child : block->get_childrens()) {
                build_node(func, child);
            }

            break;
        }

        case node_type::conditional_loop: {
            std::shared_ptr<conditional_loop_node> loop = std::dynamic_pointer_cast<conditional_loop_node>(node);
            build_conditional_loop(func, loop);

            break;
        }

        case node_type::ret: {
            build_ret(func, std::dynamic_pointer_cast<return_node>(node));
            break;
        }

        default: {
            break;
        }
        }
    }

    void ir_compiler::compile(std::shared_ptr<snack::unit_node> tar) {
        target = tar;

        header.magic[0] = 'S';
        header.magic[1] = 'N';
        header.magic[3] = 'L';
        header.magic[4] = '\0';

        ir_bin.write(reinterpret_cast<const char *>(&header), sizeof(ir_binary_header));

        code_addr = ir_bin.tellp();

        for (const auto &child : target->get_childrens()) {
            build_node(nullptr, child);
        }

        do_relocate();
        write_data_relocate_info();
        write_func_entries();
        write_unit_ref_table();

        header.code_addr = code_addr;
        header.data_addr = data_addr;
        header.func_count = funcs.size();
        header.relocate_addr = relocate_info_addr;
        header.func_table_addr = func_table_addr;
        header.ref_table_addr = ref_table_addr;
        header.ref_count = unit_table.size();

        ir_bin.seekp(0);
        ir_bin.write(reinterpret_cast<const char *>(&header), sizeof(ir_binary_header));
    }

    void ir_compiler::build_unary(function_node_ptr func, std::shared_ptr<unary_node> node) {
        build_push_hs(func, node->get_lhs());

        switch (node->get_unary_op()) {
        case caculate_op::not: {
            emit(opcode::uno);
            break;
        }

        case caculate_op::add: {
            break;
        }

        case caculate_op::sub: {
            emit(opcode::ung);
            break;
        }

        case caculate_op::reverse: {
            emit(opcode::uin);
            break;
        }

        default: {
            do_report(error_panic_code::invalid_unary, error_level::error, node);
            break;
        }
        }
    }

    void ir_compiler::build_conditional_loop(function_node_ptr func, std::shared_ptr<conditional_loop_node> node) {
        for (const auto &init_job : node->get_init_jobs()) {
            build_node(func, init_job);
        }

        std::vector<size_t> rewrite_addrs;

        size_t condition_addr = funcs.back().crr_pc;

        for (const auto &cont_condition : node->get_continue_conditions()) {
            build_condition(func, cont_condition);

            // if one of these conditions failed, it should not continue execution
            rewrite_addrs.push_back(static_cast<size_t>(ir_bin.tellp()) + 2);
            emit(opcode::brf, 0);
        }

        for (const auto &state : node->get_do_block()->get_childrens()) {
            build_node(func, state);
        }

        for (const auto &end_job : node->get_end_jobs()) {
            build_node(func, end_job);
        }

        emit(opcode::br, condition_addr);

        size_t end_do_block_pc = funcs.back().crr_pc;
        size_t last_pos = ir_bin.tellp();

        for (const auto &rewrite_addr : rewrite_addrs) {
            ir_bin.seekp(rewrite_addr);
            ir_bin.write(reinterpret_cast<const char *>(&end_do_block_pc), sizeof(size_t));
            ir_bin.seekp(last_pos);
        }
    }

    void ir_compiler::build_if_else(function_node_ptr func, std::shared_ptr<if_else_node> node) {
        build_condition(func, node->get_condition());

        size_t rewrite_addr = static_cast<size_t>(ir_bin.tellp()) + 2;
        emit(opcode::brf, 0);

        for (const auto &if_blck_stmt : node->get_if_block()->get_childrens()) {
            build_node(func, if_blck_stmt);
        }

        size_t else_block_addr = funcs.back().crr_pc;
        size_t last_pos = ir_bin.tellp();

        if (node->get_else_block()) {
            // If there is node block, there will be a br opcode that jump to the end of the else block.
            // That opcode is still belongs to if, so increase the else block address by size of br instruction.
            else_block_addr += 2 + sizeof(size_t);
        }

        ir_bin.seekp(rewrite_addr);
        ir_bin.write(reinterpret_cast<const char *>(&else_block_addr), sizeof(size_t));
        ir_bin.seekp(last_pos);

        if (node->get_else_block()) {
            size_t revist_addr = static_cast<size_t>(ir_bin.tellp()) + 2;
            emit(opcode::br, 0);

            for (const auto &else_blck_stmt : node->get_else_block()->get_childrens()) {
                build_node(func, else_blck_stmt);
            }

            size_t else_block_end_addr = funcs.back().crr_pc;
            last_pos = ir_bin.tellp();

            ir_bin.seekp(revist_addr);
            ir_bin.write(reinterpret_cast<const char *>(&else_block_end_addr), sizeof(size_t));
            ir_bin.seekp(last_pos);
        }
    }

    void ir_compiler::build_condition(function_node_ptr func, node_ptr node) {
        if (node->get_node_type() == node_type::caculate) {
            size_t revisit_write = 0;
            size_t revisit_result = 0;

            std::shared_ptr<caculate_node> cn = std::dynamic_pointer_cast<caculate_node>(node);

            if (cn->get_op() != caculate_op::logical_and && cn->get_op() != caculate_op::logical_or) {
                build_caculate(func, cn);
                return;
            }

            build_push_hs(func, node->get_lhs());

            if (cn->get_op() == caculate_op::logical_and) {
                // Come the current node, if it's not good (expression is false),
                // branch to after the binary
                //emit(opcode::ldcst, 0);
                revisit_write = static_cast<size_t>(ir_bin.tellp()) + 2;
                emit(opcode::brf, 0);
            } else if (cn->get_op() == caculate_op::logical_or) {
                // Compare the current node, if it's good (expression is false),
                // branch to after the binary
                //emit(opcode::ldcst, 0);
                revisit_write = static_cast<size_t>(ir_bin.tellp()) + 2;
                emit(opcode::brt, 0);
            }

            build_push_hs(func, node->get_rhs());

            size_t last_pos = ir_bin.tellp();

            revisit_result = funcs.back().crr_pc;

            ir_bin.seekp(revisit_write);
            ir_bin.write(reinterpret_cast<const char *>(&revisit_result), sizeof(size_t));

            ir_bin.seekp(last_pos);
        } else {
            build_push_hs(func, node);
        }
    }

    void ir_compiler::build_ret(function_node_ptr func, std::shared_ptr<return_node> node) {
        build_push_hs(func, node->get_result());
        emit(opcode::ret);
    }

    std::string ir_compiler::get_compile_binary() {
        return ir_bin.str();
    }

    size_t ir_compiler::get_code_start() {
        return code_addr;
    }

    void ir_compiler::do_relocate() {
        data_addr = ir_bin.tellp();

        for (const auto &str : relocate_string_list) {
            size_t crr_pos = ir_bin.tellp();
            size_t str_len = str.first.length();

            emit(opcode::strdata);

            ir_bin.write(reinterpret_cast<const char *>(&str_len), sizeof(size_t));
            ir_bin.write(str.first.data(), str_len);

            size_t cont_pos = ir_bin.tellp();

            for (const auto &relocate : str.second) {
                ir_bin.seekp(relocate);
                ir_bin.write(reinterpret_cast<char *>(&crr_pos), sizeof(size_t));
            }

            ir_bin.seekp(cont_pos);
        }

        for (const auto &num : relocate_number_list) {
            size_t crr_pos = ir_bin.tellp();

            emit(opcode::idata);

            ir_bin.write(reinterpret_cast<const char *>(&num.first), sizeof(long double));

            size_t cont_pos = ir_bin.tellp();

            for (const auto &relocate : num.second) {
                ir_bin.seekp(relocate);
                ir_bin.write(reinterpret_cast<char *>(&crr_pos), sizeof(size_t));
            }

            ir_bin.seekp(cont_pos);
        }
    }

    void ir_compiler::write_data_relocate_info() {
        relocate_info_addr = ir_bin.tellp();

        for (const auto &relocate_addr : relocate_number_list) {
            ir_bin.write(reinterpret_cast<const char *>(&relocate_addr), sizeof(size_t));
        }

        for (const auto &relocate_addr : relocate_string_list) {
            ir_bin.write(reinterpret_cast<const char *>(&relocate_addr), sizeof(size_t));
        }
    }

    void ir_compiler::write_func_entries() {
        func_table_addr = ir_bin.tellp();

        for (const auto &func : funcs) {
            ir_bin.write(reinterpret_cast<const char *>(&func.opcodes[0].bin_addr), sizeof(size_t));
        }
    }

    void ir_compiler::write_unit_ref_table() {
        ref_table_addr = ir_bin.tellp();

        for (const auto &name : unit_table) {
            size_t len = name.length();

            ir_bin.write(reinterpret_cast<const char *>(&len), sizeof(size_t));
            ir_bin.write(name.data(), len);
        }
    }
}