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

#include <snack/token.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace snack {
    class node;
    using node_ptr = std::shared_ptr<node>;

    enum class node_type {
        root,
        caculate,
        number,
        string,
        stmt,
        expr,
        var,
        list_expr,
        type,
        unit,
        assign,
        unary,
        function,
        function_call,
        ret,
        null,
        unit_ref,
        if_else,
        block,
        conditional_loop,
        unconditional_loop,
        unknown
    };

    class node {
        friend class parser;

        node_ptr left;
        node_ptr right;
        node_type type;
        node_ptr parent;

    protected:
        size_t line;
        size_t column;

    public:
        explicit node();
        node(node_ptr parent, node_type type, token tok);

        virtual ~node() {}

        node_ptr get_lhs();
        node_ptr get_rhs();
        node_ptr get_parent();

        node_type get_node_type() const {
            return type;
        }

        size_t get_line() const {
            return line;
        }

        size_t get_column() const {
            return column;
        }
    };

    enum class caculate_op {
        and,
        or,
        xor,
        sub,
        add,
        mul,
        div,
        mod,
        power,
        shl,
        shr,
        not,
        reverse,
        equal,
        greater,
        greater_equal,
        less,
        logical_and,
        logical_or,
        logical_xor,
        less_equal
    };

    class string_node : public node {
        std::string str;

    public:
        string_node(node_ptr parent, const std::string &nstr);
        const std::string &get_string() const;
    };

    class number_node : public node {
        long double val;

    public:
        number_node(node_ptr parent, const long double val);
        const long double get_value() const;
    };

    class stmt_node : public node {
        std::vector<node_ptr> childrens;

    protected:
        stmt_node(node_ptr parent, node_type type, token tok);

    public:
        stmt_node(node_ptr parent, token tok);

        void add_children(node_ptr node);

        std::vector<node_ptr> &get_childrens() {
            return childrens;
        }
    };

    class expr_node : public stmt_node {
    protected:
        expr_node(node_ptr parent, node_type type, token tok);

    public:
        expr_node(node_ptr parent, token tok);
    };

    class unit_reference_node;

    class unit_node : public stmt_node {
        std::unordered_map<long double, std::shared_ptr<number_node>> numbers;
        std::unordered_map<std::string, std::shared_ptr<string_node>> strings;

        std::vector<std::shared_ptr<unit_reference_node>> unit_refs;

        friend class parser;

    public:
        unit_node(node_ptr parent);

        std::unordered_map<long double, std::shared_ptr<number_node>> &get_number_map() {
            return numbers;
        }

        std::unordered_map<std::string, std::shared_ptr<string_node>> &get_string_map() {
            return strings;
        }
    };

    enum class var_type {
        number,
        string,
        none,
        undefined
    };

    class type_node : public node {
        var_type type;
        friend class parser;

    public:
        type_node(node_ptr parent);

        var_type get_var_type() const {
            return type;
        }
    };

    class var_node : public node {
        std::string var_name;
        std::shared_ptr<type_node> var_type;

        friend class parser;

    public:
        var_node(node_ptr parent, token tok);
        var_node(node_ptr parent, token tok, const std::string &variable_name);

        const std::string &get_var_name() const {
            return var_name;
        }

        std::shared_ptr<type_node> get_type_node() {
            return var_type;
        }
    };

    class unary_node : public expr_node {
        caculate_op unary_op;

        friend class parser;

    public:
        unary_node(node_ptr parent, token tok);

        node_ptr get_unary_target() {
            return get_lhs();
        }

        caculate_op get_unary_op() const {
            return unary_op;
        }
    };

    class assign_node : public stmt_node {
    public:
        assign_node(node_ptr parent, token tok);
    };

    class caculate_node : public expr_node {
        caculate_op op;

        friend class parser;

    public:
        caculate_node(node_ptr parent, token tok);
        caculate_node(node_ptr parent, token tok, caculate_op op);

        caculate_op get_op() const;
    };

    using var_node_ptr = std::shared_ptr<var_node>;

    enum class function_type {
        full_link,
        external
    };

    class function_node : public stmt_node {
        std::shared_ptr<type_node> ret_type;

        std::vector<var_node_ptr> args;
        std::vector<var_node_ptr> local_vars;

        std::string name;

        function_type func_type = function_type::full_link;

        friend class parser;

    public:
        function_node(node_ptr parent, token tok);

        const std::string &get_name() const {
            return name;
        }

        std::vector<var_node_ptr> &get_args() {
            return args;
        }

        std::vector<var_node_ptr> &get_local_vars() {
            return local_vars;
        }
    };

    using function_node_ptr = std::shared_ptr<function_node>;

    class function_call_node : public stmt_node {
        function_node_ptr func;
        std::vector<node_ptr> args_passed;

        friend class parser;

    public:
        function_call_node(node_ptr node, token tok);

        function_node_ptr get_function() {
            return func;
        }

        std::vector<node_ptr> &get_args() {
            return args_passed;
        }
    };

    class null_node : public node {
    public:
        null_node(node_ptr parent, token tok);
    };

    class return_node : public stmt_node {
        node_ptr return_expr;

        friend class parser;

    public:
        return_node(node_ptr parent, token tok);
    };

    class unit_reference_node : public stmt_node {
        std::string unit_name;
        friend class parser;

    public:
        unit_reference_node(node_ptr parent, token tok);

        const std::string &get_unit_ref_name() const {
            return unit_name;
        }
    };

    class block_node : public stmt_node {
    public:
        block_node(node_ptr parent, token tok);
    };

    class if_else_node : public stmt_node {
        node_ptr condition;

        std::shared_ptr<block_node> if_block;
        std::shared_ptr<block_node> else_block;

        friend class parser;

    public:
        if_else_node(node_ptr parent, token tok);

        node_ptr get_condition() {
            return condition;
        }

        std::shared_ptr<block_node> get_if_block() {
            return if_block;
        }

        std::shared_ptr<block_node> get_else_block() {
            return else_block;
        }
    };

    class conditional_loop_node : public stmt_node {
        std::vector<node_ptr> continue_conditions;

        std::vector<node_ptr> end_jobs;
        std::vector<node_ptr> init_jobs;

        std::shared_ptr<block_node> do_block;

        friend class parser;

    public:
        conditional_loop_node(node_ptr parent, token tok);

        std::shared_ptr<block_node> get_do_block() {
            return do_block;
        }

        std::vector<node_ptr> &get_continue_conditions() {
            return continue_conditions;
        }

        std::vector<node_ptr> &get_end_jobs() {
            return end_jobs;
        }

        std::vector<node_ptr> &get_init_jobs() {
            return init_jobs;
        }
    };
}