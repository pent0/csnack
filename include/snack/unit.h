#pragma once

#include <snack/ast.h>
#include <snack/ir_interpreter.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace snack::ir::backend {
    class ir_interpreter;
}

namespace snack::userspace {
    class unit {
    public:
        virtual ~unit() {}

        virtual bool call_function(ir::backend::ir_interpreter *interpreter, const uint8_t idx,
            ir::backend::ir_interpreter_func_context *context)
            = 0;
        virtual std::optional<size_t> get_function_idx(const std::string &name, size_t arg_count) = 0;

        virtual const std::string &get_unit_name() const = 0;
        virtual std::optional<std::string> get_reference_unit_name(const uint8_t idx) = 0;
    };

    using unit_ptr = std::shared_ptr<unit>;
}

namespace snack::userspace {
    class external_unit : public userspace::unit {
    protected:
        using function = std::function<void(ir::backend::ir_interpreter_func_context &)>;

        struct func_info {
            function real_func;
            std::string name;
            int16_t arg_count;

            func_info(function func, const std::string &name, int16_t arg_count)
                : real_func(func)
                , name(name) 
                , arg_count(arg_count) {}
        };

    private:

        std::string name;

    protected:
        std::vector<func_info> functions;

    public:
        external_unit() {}
        external_unit(const std::string &unit_name);

        ~external_unit() {}

        bool call_function(ir::backend::ir_interpreter *interpreter, const uint8_t idx,
            ir::backend::ir_interpreter_func_context *context) override;

        const std::string &get_unit_name() const override {
            return name;
        }

        std::optional<size_t> get_function_idx(const std::string &name, size_t arg_count) override;

        std::optional<std::string> get_reference_unit_name(const uint8_t idx) override {
            return std::optional<std::string>{};
        }
    };

    class interpreted_unit : public userspace::unit {  
        struct interpreted_unit_func_info {
            std::string name;
            size_t addr;
            size_t arg_count;
        };

        const char *ir_bin;
        std::string name;

        std::vector<interpreted_unit_func_info> functions;
        std::vector<std::string> unit_ref_names;

    protected:
        void query_entries();

    public:
        interpreted_unit() {}
        interpreted_unit(const std::string &unit_name, const char *ir_bin);

        ~interpreted_unit() {}

        bool call_function(ir::backend::ir_interpreter *interpreter, const uint8_t idx,
            ir::backend::ir_interpreter_func_context *context) override;

        const std::string &get_unit_name() const override {
            return name;
        }

        std::optional<size_t> get_function_idx(const std::string &name, size_t arg_count) override;
        std::optional<std::string> get_reference_unit_name(const uint8_t idx) override;
    };

#define REGISTER_UNIT_FUNC(unit, name, func, arg_count) functions.push_back(external_unit::func_info{ std::bind(&unit::##func, this, std::placeholders::_1), name, arg_count })
}