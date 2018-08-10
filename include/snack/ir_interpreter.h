#pragma once

#include <snack/error.h>
#include <snack/ir_opcode.h>

#include <array>
#include <functional>
#include <stack>
#include <string>
#include <unordered_map>

#include <sstream>

namespace snack {
    namespace userspace {
        class interpreted_unit;
        class external_unit;

        class unit_manager;
        class unit;
    }
}

namespace snack::ir::backend {
    struct ir_element {
        enum {
            none,
            num,
            str
        } type;

        std::string str_data;
        long double num_data;

        std::string associated_name;

        ir_element()
            : num_data(0)
            , str_data("")
            , type(none) {}

        ir_element(const std::string &str_data)
            : str_data(str_data)
            , type(str) {}

        ir_element(const long double num_data)
            : num_data(num_data)
            , type(num) {}
    };

    struct ir_interpreter_func_context {
        std::array<ir_element, 200> local_slots;
        std::stack<ir_element> evaluation_stack;
        std::array<ir_element, 30> local_args;

        const char *ir_bin;
        const char *ir_global_bin;

        std::string func_name;
        userspace::unit *owning_unit;

        size_t pc;
    };

    class ir_interpreter {
        std::stack<ir_interpreter_func_context> func_contexts;
        std::unordered_map<ir::opcode, std::function<void(ir_interpreter_func_context &)>> opcode_map;

        snack::userspace::unit_manager *unit_mngr;
        snack::error_manager *err_manager;

        ir_interpreter_func_context *last_context;

        friend class userspace::interpreted_unit;
        friend class userspace::external_unit;

    protected:
        ir_interpreter_func_context &get_current_func_context();

        void ldcst(ir_interpreter_func_context &context);
        void ldlc(ir_interpreter_func_context &context);
        void ldcststr(ir_interpreter_func_context &context);
        void strlc(ir_interpreter_func_context &context);

        void add(ir_interpreter_func_context &context);
        void sub(ir_interpreter_func_context &context);
        void mul(ir_interpreter_func_context &context);
        void div(ir_interpreter_func_context &context);
        void mod(ir_interpreter_func_context &context);
        void shl(ir_interpreter_func_context &context);
        void shr(ir_interpreter_func_context &context);
        void pwr(ir_interpreter_func_context &context);

        void ceq(ir_interpreter_func_context &context);
        void cgt(ir_interpreter_func_context &context);
        void cge(ir_interpreter_func_context &context);
        void clt(ir_interpreter_func_context &context);
        void cle(ir_interpreter_func_context &context);

        void ble(ir_interpreter_func_context &context);
        void blt(ir_interpreter_func_context &context);
        void beq(ir_interpreter_func_context &context);
        void bge(ir_interpreter_func_context &context);
        void bgt(ir_interpreter_func_context &context);
        void br(ir_interpreter_func_context &context);
        void brf(ir_interpreter_func_context &context);
        void brt(ir_interpreter_func_context &context);

        void uno(ir_interpreter_func_context &context);
        void ung(ir_interpreter_func_context &context);
        void uin(ir_interpreter_func_context &context);

        void met(ir_interpreter_func_context &context);
        void endmet(ir_interpreter_func_context &context);
        void vri(ir_interpreter_func_context &context);
        void vrs(ir_interpreter_func_context &context);
        void call(ir_interpreter_func_context &context);

        void pop(ir_interpreter_func_context &context);

        void do_opcode();

        void init_opcode_table();

    public:
        explicit ir_interpreter(snack::error_manager &err_mngr, snack::userspace::unit_manager &manager);
        void interpret();
    };
}