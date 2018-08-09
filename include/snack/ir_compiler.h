#pragma once

#include <snack/ast.h>
#include <snack/error.h>
#include <snack/ir_opcode.h>
#include <snack/unit_manager.h>

#include <memory>
#include <sstream>
#include <unordered_map>

namespace snack::ir::backend {
    struct ir_op_info {
        opcode op;
        size_t bin_addr;
        size_t pc_context_addr;
    };

    struct ir_function {
        std::vector<ir_op_info> opcodes;

        size_t crr_pc;
    };

    struct ir_binary_header {
        char magic[4];
        char major;
        char minor;
        char build;
        size_t code_addr;
        size_t data_addr;
        size_t relocate_addr;
        size_t func_count;
        size_t func_table_addr;
        size_t ref_count;
        size_t ref_table_addr;
    };

    class ir_compiler {
        std::shared_ptr<snack::unit_node> target;
        std::stringstream ir_bin;

        std::vector<ir_function> funcs;

        std::unordered_map<std::string, std::vector<size_t>> relocate_string_list;
        std::unordered_map<long double, std::vector<size_t>> relocate_number_list;

        std::vector<std::string> unit_table;

        ir_binary_header header;

        size_t data_addr;
        size_t code_addr;
        size_t relocate_info_addr;
        size_t func_table_addr;
        size_t ref_table_addr;

        snack::error_manager *err_mngr;
        snack::userspace::unit_manager *unit_mngr;

    protected:
        void write_header();
        void write_data_relocate_info();
        void do_relocate();
        void write_func_entries();
        void write_unit_ref_table();

        void emit(opcode op);
        void emit(opcode op, long double value);
        void emit(opcode op, const std::string &value);
        void emit(opcode op, long double nval, const std::string &sval);

        void build_function(function_node_ptr node);
        void build_caculate(function_node_ptr func, std::shared_ptr<caculate_node> node);
        void build_assign(function_node_ptr func, std::shared_ptr<assign_node> node);
        void build_function_call(function_node_ptr func, std::shared_ptr<function_call_node> fun);
        void build_if_else(function_node_ptr func, std::shared_ptr<if_else_node> node);

        void build_condition(function_node_ptr func, node_ptr node);
        void build_node(function_node_ptr func, node_ptr node);
        void build_push_hs(function_node_ptr func, node_ptr node);

        void do_report(error_panic_code code, error_level level, node_ptr node);
        void do_report(error_panic_code code, error_level level, node_ptr node, const std::string &arg0);
        void do_report(error_panic_code code, error_level level, node_ptr node, const std::string &arg0, const std::string &arg1);

    public:
        explicit ir_compiler() {}
        ir_compiler(snack::error_manager &mngr, snack::userspace::unit_manager &unit_mngr);

        void compile(std::shared_ptr<snack::unit_node> tar);

        std::string get_compile_binary();
        size_t get_code_start();
    };
}