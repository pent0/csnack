/*
 * Copyright (c) 2018 EKA2L1 Team.
 * 
 * This file is part of EKA2L1 project 
 * (see bentokun.github.com/EKA2L1).
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <snack/ast.h>
#include <snack/error.h>
#include <snack/lexer.h>

namespace snack {
    using type_node_ptr = std::shared_ptr<type_node>;
    using var_node_ptr = std::shared_ptr<var_node>;
    using function_node_ptr = std::shared_ptr<function_node>;
    using number_node_ptr = std::shared_ptr<number_node>;
    using string_node_ptr = std::shared_ptr<string_node>;
    using unit_node_ptr = std::shared_ptr<unit_node>;
    using unit_reference_node_ptr = std::shared_ptr<unit_reference_node>;

    class parser {
        lexer code_lexer;
        unit_node_ptr unit;

        error_manager *err_manager;

        std::vector<var_node_ptr> vars;
        std::vector<function_node_ptr> functions;
        std::vector<unit_reference_node_ptr> unit_used;

    protected:
        std::shared_ptr<node> parse_expr(node_ptr parent);
        std::shared_ptr<node> parse_rhs(node_ptr parent);

        std::shared_ptr<node> parse_factor(node_ptr parent);
        std::shared_ptr<node> parse_term(node_ptr parent);

        std::shared_ptr<node> parse_stmt(node_ptr parent);
        std::shared_ptr<node> parse_function(node_ptr parent);
        std::shared_ptr<node> parse_return(node_ptr parent);

        std::shared_ptr<var_node> parse_var(node_ptr parent);
        std::shared_ptr<if_else_node> parse_if_else(node_ptr parent);
        std::shared_ptr<node> parse_do_chain(node_ptr parent);

        std::shared_ptr<assign_node> parse_assign_node(node_ptr parent, var_node_ptr lhs);
        std::shared_ptr<function_call_node> parse_function_call(node_ptr parent);

        type_node_ptr make_undefined_type();
        number_node_ptr make_number(const long double num);
        string_node_ptr make_string(const std::string &str);

        var_node_ptr get_var(node_ptr parent, const std::string &ident_name);
        function_node_ptr get_function(const std::string &func_name, size_t arg_count);

        void do_parsing();

        void do_report(int error_code, error_level level, token tok);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1,
            const std::string &arg2);

    public:
        parser(error_manager &err_mngr, lexer &lex);
        void parse();

        unit_node_ptr get_unit_node();
    };
}