#include <snack/ir_compiler.h>
#include <snack/ir_interpreter.h>
#include <snack/unit.h>

namespace snack::userspace {
    interpreted_unit::interpreted_unit(const std::string &unit_name, const char *ir_bin)
        : ir_bin(ir_bin)
        , name(unit_name) {
        query_entries();
    }

    void interpreted_unit::query_entries() {
        const ir::backend::ir_binary_header *header = reinterpret_cast<decltype(header)>(ir_bin);

        for (size_t i = 0; i < header->func_count; i++) {
            const size_t addr = *reinterpret_cast<const size_t *>(ir_bin + header->func_table_addr + i * sizeof(size_t));
            const uint16_t arg_count = *reinterpret_cast<const size_t *>(ir_bin + addr + 2);
            const size_t name_addr = *reinterpret_cast<const size_t *>(ir_bin + addr + 4);

            const size_t name_len = *reinterpret_cast<const size_t *>(ir_bin + name_addr + 2);

            std::string name;
            name.resize(name_len);

            std::copy(ir_bin + name_addr + 2 + sizeof(size_t), ir_bin + name_addr + 2 + sizeof(size_t) + name_len, name.begin());

            functions.push_back(interpreted_unit_func_info{ name, addr, arg_count });
        }

        size_t ref_pc = header->ref_table_addr;

        for (size_t i = 0; i < header->ref_count; i++) {
            size_t str_len = *reinterpret_cast<const size_t *>(ref_pc + ir_bin);
            ref_pc += sizeof(str_len);

            std::string unit_name;
            unit_name.resize(str_len);

            std::copy(ir_bin + ref_pc, ir_bin + ref_pc + str_len, unit_name.begin());
            ref_pc += str_len;

            unit_ref_names.push_back(unit_name);
        }
    }

    std::optional<size_t> interpreted_unit::get_function_idx(const std::string &name, size_t arg_count) {
        const auto &func = std::find_if(functions.begin(), functions.end(), 
            [&](interpreted_unit_func_info &info) { return info.name ==name && info.arg_count ==arg_count; });

        if (func == functions.end()) {
            return std::optional<size_t>{};
        }

        return func - functions.begin();
    }

    bool interpreted_unit::call_function(ir::backend::ir_interpreter *interpreter, const uint8_t idx,
        ir::backend::ir_interpreter_func_context *context) {
        if (idx >= functions.size()) {
            return false;
        }

        ir::backend::ir_interpreter_func_context function;

        function.pc = 0;
        function.func_name = functions[idx].name;
        function.ir_bin = ir_bin + functions[idx].addr;
        function.ir_global_bin = ir_bin;
        function.owning_unit = this;
        function.ref_manager = interpreter->get_ref_manager();

        interpreter->func_contexts.push(std::move(function));

        return true;
    }

    std::optional<std::string> interpreted_unit::get_reference_unit_name(const uint8_t idx) {
        if (idx >= unit_ref_names.size()) {
            return std::optional<std::string>{};
        }

        return unit_ref_names[idx];
    }

    external_unit::external_unit(const std::string &unit_name)
        : name(unit_name) {
    }

    bool external_unit::call_function(ir::backend::ir_interpreter *interpreter, const uint8_t idx,
        ir::backend::ir_interpreter_func_context *context) {
        if (idx >= functions.size()) {
            return false;
        }

        auto func = functions[idx].real_func;

        func(*context);

        return true;
    }

    std::optional<size_t> external_unit::get_function_idx(const std::string &name, size_t arg_count) {
        const auto &func = std::find_if(functions.begin(), functions.end(),
            [&](func_info &inf) { return (inf.name == name) && (inf.arg_count == -1 || (inf.arg_count ==arg_count)); });

        if (func == functions.end()) {
            return std::optional<size_t>{};
        }

        return func - functions.begin();
    }
}