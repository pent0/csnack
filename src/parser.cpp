#include <snack/parser.h>

#include <algorithm>

namespace snack {
    parser::parser(error_manager &err_mngr, lexer &lex)
        : code_lexer(std::move(lex))
        , err_manager(&err_mngr) {
        unit = std::make_shared<unit_node>(nullptr);
    }

    unit_node_ptr parser::get_unit_node() {
        return unit;
    }

    void parser::do_report(int error_code, error_level level, token tok) {
        if (err_manager) {
            err_manager->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row());
        }
    }

    void parser::do_report(int error_code, error_level level, token tok, const std::string &arg0) {
        if (err_manager) {
            err_manager->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row(), arg0);
        }
    }

    void parser::do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1,
        const std::string &arg2) {
        if (err_manager) {
            std::string temp_arr[] = { arg0, arg1, arg2 };

            err_manager->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row(), temp_arr, 3);
        }
    }

    void parser::do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1) {
        if (err_manager) {
            err_manager->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row(), arg0, arg1);
        }
    }

    type_node_ptr parser::make_undefined_type() {
        type_node_ptr new_type = std::make_shared<type_node>(unit);
        new_type->type = var_type::undefined;

        return new_type;
    }

    number_node_ptr parser::make_number(const long double num) {
        auto res = unit->numbers.find(num);

        if (res == unit->numbers.end()) {
            number_node_ptr new_num = std::make_shared<number_node>(unit, num);
            unit->numbers.emplace(num, std::move(new_num));

            return unit->numbers[num];
        }

        return res->second;
    }

    string_node_ptr parser::make_string(const std::string &str) {
        auto res = unit->strings.find(str);

        if (res == unit->strings.end()) {
            string_node_ptr new_str = std::make_shared<string_node>(unit, str);
            unit->strings.emplace(str, std::move(new_str));

            return unit->strings[str];
        }

        return res->second;
    }

    var_node_ptr parser::get_var(node_ptr parent, const std::string &ident_name) {
        // traverse node until get
        node_ptr crr = parent;

        while (crr && crr->type != node_type::function) {
            crr = crr->parent;
        }

        if (crr && crr->type == node_type::function) {
            function_node_ptr func = std::dynamic_pointer_cast<function_node>(crr);

            auto &res = std::find_if(func->local_vars.begin(), func->local_vars.end(),
                [&](var_node_ptr var) { return var->var_name == ident_name; });

            if (res == func->local_vars.end()) {
                auto &res = std::find_if(vars.begin(), vars.end(),
                    [&](var_node_ptr var) { return var->var_name == ident_name; });

                if (res == vars.end()) {
                    return nullptr;
                }

                return *res;
            }

            return *res;
        } else {
            auto &res = std::find_if(vars.begin(), vars.end(),
                [&](var_node_ptr var) { return var->var_name == ident_name; });

            if (res == vars.end()) {
                return nullptr;
            }

            return *res;
        }

        return nullptr;
    }

    function_node_ptr parser::get_function(const std::string &func_name, size_t arg_count) {
        auto &res = std::find_if(functions.begin(), functions.end(),
            [&](function_node_ptr func) { return (func->get_name() == func_name) && (func->get_args().size() == arg_count); });

        if (res == functions.end()) {
            // Make external function
            function_node_ptr func = std::make_shared<function_node>(unit, token{});
            func->name = func_name;
            func->func_type = function_type::external;
            func->args.resize(arg_count);

            functions.push_back(func);

            return functions.back();
        }

        return *res;
    }

    std::shared_ptr<node> parser::parse_stmt(node_ptr parent) {
        std::optional<token> tok = code_lexer.peek();
        if (!tok) {
            return nullptr;
        }

        switch (tok->get_token_type()) {
        // support mixing keyword
        case token_type::ident:
        case token_type::keyword: {
            std::string tok_val = tok->get_raw_token_string();

            if (tok_val == "var") {
                auto new_var = parse_var(parent);

                if (code_lexer.peek()->get_raw_token_string() != "=") {
                    do_report(error_panic_code::var_unintialized, error_level::warn,
                        *tok);

                    return new_var;
                }

                // parse assignment(ident)
                // if assignment node is null, than throw error
                return parse_assign_node(parent, new_var);
            } else if (tok_val == "fn") {
                return parse_function(parent);
            } else if (tok_val == "ret") {
                return parse_return(parent);
            } else if (tok_val == "if") {
                return parse_if_else(parent);
            } else if (tok_val == "do") {
                return parse_do_chain(parent);
            } else if (tok_val == "for" || tok_val == "while") {
                return parse_conditional_loop(parent);
            } else if (tok_val == "uses") {
                if (parent->get_node_type() != node_type::unit) {
                    do_report(error_panic_code::invalid_use, snack::error_level::error,
                        *tok);

                    return nullptr;
                }

                unit_reference_node_ptr ref = std::make_shared<unit_reference_node>(parent, *tok);
                code_lexer.next();
                code_lexer.next();

                auto name_tok = code_lexer.get_current_token();

                if (!name_tok || name_tok->get_token_type() != token_type::ident) {
                    do_report(error_panic_code::expect_got, snack::error_level::error,
                        *tok, tok->get_raw_token_string());
                }

                ref->unit_name = code_lexer.get_current_token()->get_raw_token_string();
                unit->unit_refs.push_back(std::move(ref));

                return unit->unit_refs.back();
            } else {
                // Peek to see if this is an assign expression
                code_lexer.next();

                auto tok_p = code_lexer.peek();
                node_ptr lhs = get_var(parent, tok->get_raw_token_string());

                if (!lhs) {
                    if (tok_p && tok_p->get_raw_token_string() == "(") {
                        return parse_function_call(parent);
                    }
                    // Error
                    do_report(error_panic_code::var_not_found, snack::error_level::error,
                        *tok, tok->get_raw_token_string());

                    return nullptr;
                }

                if (code_lexer.peek()->get_raw_token_string() == "[") {
                    std::shared_ptr<array_access_node> access = std::make_shared<array_access_node>(parent,
                        *code_lexer.get_current_token());

                    code_lexer.next();

                    access->index = parse_expr(access);

                    if (!access->index) {
                        // report
                        return nullptr;
                    }

                    access->var = std::move(lhs);
                    lhs = std::move(access);

                    if (code_lexer.peek()->get_raw_token_string() != "]") {
                        // do report
                        return nullptr;
                    }

                    code_lexer.next();
                }

                if (code_lexer.peek()->get_raw_token_string() == "=" || (code_lexer.peek()->get_raw_token_string().length() == 2 && code_lexer.peek()->get_raw_token_string()[1] == '='))
                    return parse_assign_node(parent, lhs);

                code_lexer.back();

                node_ptr ret = parse_expr(parent);
                return ret;
            }
        }

        default:
            break;
        }

        do_report(error_panic_code::stmt_unable_parse, error_level::error, *tok);

        return nullptr;
    }

    std::shared_ptr<node> parser::parse_ident_based(node_ptr parent) {
        auto tok = code_lexer.get_current_token();

        if (tok->get_raw_token_string() == "new") {
            return nullptr;
        }

        // Peek to see if this is an assign expression
        code_lexer.next();

        node_ptr lhs = get_var(parent, tok->get_raw_token_string());

        if (!lhs) {
            auto tok_p = code_lexer.get_current_token();

            if (tok_p && tok_p->get_raw_token_string() == "(") {
                code_lexer.back();
                return parse_function_call(parent);
            }
            // Error
            do_report(error_panic_code::var_not_found, snack::error_level::error,
                *tok, tok->get_raw_token_string());

            return nullptr;
        }

        code_lexer.back();

        if (code_lexer.peek()->get_raw_token_string() == "[") {
            std::shared_ptr<array_access_node> access = std::make_shared<array_access_node>(parent,
                *code_lexer.get_current_token());

            code_lexer.next();

            access->index = parse_expr(access);

            if (!access->index) {
                // report
                return nullptr;
            }

            access->var = std::move(lhs);
            lhs = std::move(access);

            if (code_lexer.peek()->get_raw_token_string() != "]") {
                // do report
                return nullptr;
            }

            code_lexer.next();
        }

        return lhs;
    }

    std::shared_ptr<node> parser::parse_rhs(node_ptr parent) {
        int org_pos = code_lexer.get_pointer();

        std::shared_ptr<node> res = parse_expr(parent);

        if (!res) {
            code_lexer.jump(org_pos);
            auto tok = code_lexer.peek();

            if (tok->get_token_type() == token_type::number) {
                res = std::move(make_number(std::stod(tok->get_raw_token_string())));
                code_lexer.next();
            } else if (tok->get_token_type() == token_type::ident) {
                res = parse_ident_based(parent);
                code_lexer.next();
            } else if (tok->get_token_type() == token_type::string) {
                res = std::move(make_string(tok->get_raw_token_string()));
                code_lexer.next();
            } else {
                // throw error
                do_report(error_panic_code::stmt_unable_parse, error_level::error, *tok);

                return nullptr;
            }
        } else {
            code_lexer.next();
        }

        return res;
    }

    std::shared_ptr<node> parser::parse_factor(node_ptr parent) {
        code_lexer.next();

        std::optional<token> tok = code_lexer.get_current_token();

        switch (tok->get_token_type()) {
        case token_type::op: {
            if (tok->get_raw_token_string() == "+") {
                std::shared_ptr<unary_node> unary = std::make_shared<unary_node>(parent, *tok);

                unary->unary_op = caculate_op::add;
                unary->left = parse_rhs(unary); // We parse the right handside, it's number

                code_lexer.back();

                return unary;
            } else if (tok->get_raw_token_string() == "-") {
                std::shared_ptr<unary_node> unary = std::make_shared<unary_node>(parent, *tok);

                unary->unary_op = caculate_op::sub;
                unary->left = parse_rhs(unary); // We parse the right handside, it's number

                code_lexer.back();

                return unary;
            } else if (tok->get_raw_token_string() == "~") {
                std::shared_ptr<unary_node> unary = std::make_shared<unary_node>(parent, *tok);

                unary->unary_op = caculate_op::reverse;
                unary->left = parse_rhs(unary); // We parse the right handside, it's number

                code_lexer.back();

                return unary;
            } else if (tok->get_raw_token_string() == "!") {
                std::shared_ptr<unary_node> unary = std::make_shared<unary_node>(parent, *tok);

                unary->unary_op = caculate_op::not;
                unary->left = parse_rhs(unary); // We parse the right handside, it's number

                code_lexer.back();

                return unary;
            }

            return nullptr;
        }

        case token_type::number: {
            return make_number(std::stod(tok->get_raw_token_string()));
        }

        case token_type::ident:
        case token_type::keyword: {
            return parse_ident_based(parent);
        }

        case token_type::string: {
            return make_string(tok->get_raw_token_string());
        }

        case token_type::parentheses: {
            if (tok->get_raw_token_string() != "(") {
                //throw expected got
                do_report(error_panic_code::expect_got, error_level::error, *code_lexer.get_last_token(),
                    "'('", tok->get_raw_token_string());

                return nullptr;
            }

            auto res = parse_rhs(parent);

            tok = code_lexer.get_current_token();

            if (tok && tok->get_raw_token_string() != ")") {
                // throw expected error
                do_report(error_panic_code::expect_got, error_level::error, *code_lexer.get_last_token(),
                    "')'", tok->get_raw_token_string());

                return nullptr;
            }

            return res;
        }
        }

        // throw error
        do_report(error_panic_code::expect_after_got, error_level::error, *code_lexer.get_last_token(),
            "an expression", code_lexer.get_last_token()->get_raw_token_string(), tok->get_raw_token_string());

        return nullptr;
    }

    std::shared_ptr<node> parser::parse_term(node_ptr parent) {
        auto factor = parse_factor(parent);

        std::shared_ptr<caculate_node> calc_node_last = nullptr;
        std::shared_ptr<caculate_node> calc_node = nullptr;

        auto tok = code_lexer.peek();

        while (tok && tok->get_token_type() == token_type::op) {
            caculate_op op;

            if (tok->get_raw_token_string() == "*") {
                op = caculate_op::mul;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "/") {
                op = caculate_op::div;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "%") {
                op = caculate_op::mod;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "&") {
                op = caculate_op::and;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "&&") {
                op = caculate_op::logical_and;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "**") {
                op = caculate_op::power;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == ">>") {
                op = caculate_op::shr;
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "<<") {
                op = caculate_op::shl;
                code_lexer.next();
            } else {
                break;
            }

            if (calc_node)
                calc_node_last = std::move(calc_node);

            calc_node = std::make_shared<caculate_node>(parent, *tok);

            if (calc_node_last)
                calc_node->left = std::move(calc_node_last);
            else
                calc_node->left = std::move(factor);

            calc_node->op = op;
            calc_node->right = std::move(parse_factor(calc_node));

            tok = code_lexer.peek();
        }

        if (!calc_node) {
            return factor;
        }

        return calc_node;
    }

    std::shared_ptr<node> parser::parse_expr(node_ptr parent) {
        auto term = parse_term(parent);

        std::shared_ptr<caculate_node> calc_node_last = nullptr;
        std::shared_ptr<caculate_node> calc_node = nullptr;

        while (code_lexer.peek() && code_lexer.peek()->get_token_type() == token_type::op) {
            token tok = *code_lexer.peek();

            caculate_op op;

            // TODO: Get rid elseif
            if (tok.get_raw_token_string() == "+") {
                op = caculate_op::add;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "-") {
                op = caculate_op::sub;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "|") {
                op = caculate_op:: or ;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "^") {
                op = caculate_op:: xor ;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "||") {
                op = caculate_op::logical_or;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "==") {
                op = caculate_op::equal;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "!=") {
                op = caculate_op::not;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == ">=") {
                op = caculate_op::greater_equal;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "<=") {
                op = caculate_op::less_equal;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == ">") {
                op = caculate_op::greater;
                code_lexer.next();
            } else if (tok.get_raw_token_string() == "<") {
                op = caculate_op::less;
                code_lexer.next();
            } else {
                break;
            }

            if (calc_node)
                calc_node_last = std::move(calc_node);

            calc_node = std::make_shared<caculate_node>(parent, tok);

            if (calc_node_last)
                calc_node->left = std::move(calc_node_last);
            else
                calc_node->left = std::move(term);

            calc_node->op = op;
            calc_node->right = std::move(parse_term(calc_node));
        }

        if (!calc_node) {
            return term;
        }

        return calc_node;
    }

    std::shared_ptr<if_else_node> parser::parse_if_else(node_ptr parent) {
        code_lexer.next();
        std::shared_ptr<if_else_node> ie = std::make_shared<if_else_node>(parent, *code_lexer.get_current_token());

        ie->condition = parse_expr(ie);

        if (!ie) {
            return nullptr;
        }

        code_lexer.next();
        auto tok = code_lexer.get_current_token();

        if (tok && tok->get_token_type() != token_type::colon) {
            do_report(error_panic_code::expect_got, error_level::error, *tok, "':'", tok->get_raw_token_string());
            return nullptr;
        }

        ie->if_block = parse_block(ie);

        if (!ie->if_block) {
            return nullptr;
        }

        if (code_lexer.peek() && code_lexer.peek()->get_raw_token_string() == "else") {
            code_lexer.next();

            code_lexer.next();
            tok = code_lexer.get_current_token();

            if (tok && tok->get_token_type() != token_type::colon) {
                do_report(error_panic_code::expect_got, error_level::error, *tok, "':'", tok->get_raw_token_string());
                return nullptr;
            }

            ie->else_block = parse_block(ie);
        }

        code_lexer.next();

        return ie;
    }

    std::shared_ptr<block_node> parser::parse_block(node_ptr parent) {
        std::shared_ptr<block_node> do_block = std::make_shared<block_node>(parent, *code_lexer.get_last_token());
        auto tok = code_lexer.peek();

        if (tok && tok->get_token_type() != token_type::indent) {
            // Get the do token, since the next token maybe in the line under
            do_report(error_panic_code::indentation_needed, error_level::error, *code_lexer.get_last_token());
            return nullptr;
        }

        code_lexer.next();
        tok = code_lexer.peek();

        while (tok && tok->get_token_type() != token_type::dedent && tok->get_token_type() != token_type::eof) {
            auto do_block_stmt = parse_stmt(do_block);

            if (do_block_stmt) {
                do_block->add_children(do_block_stmt);
            }

            tok = code_lexer.peek();
        }

        return do_block;
    }

    std::shared_ptr<node> parser::parse_do_chain(node_ptr parent) {
        code_lexer.next();

        code_lexer.next();
        auto tok = code_lexer.get_current_token();

        if (tok && tok->get_token_type() != token_type::colon) {
            // Get the do token, since the next token maybe in the line under
            do_report(error_panic_code::expect_after, error_level::error, *code_lexer.get_last_token(), "':'", "'do'",
                tok->get_raw_token_string());

            return nullptr;
        }

        std::shared_ptr<block_node> do_block = parse_block(parent);

        if (!do_block) {
            return nullptr;
        }

        code_lexer.next();
        tok = code_lexer.peek();

        if (tok->get_raw_token_string() == "unless") {
            std::shared_ptr<if_else_node> con_node = std::make_shared<if_else_node>(parent, *tok);
            std::shared_ptr<unary_node> real_condition = std::make_shared<unary_node>(con_node, *tok);

            code_lexer.next();

            real_condition->left = std::move(parse_expr(real_condition));
            real_condition->unary_op = caculate_op::not;

            con_node->condition = std::move(real_condition);
            con_node->if_block = std::move(do_block);

            return con_node;
        }

        return do_block;
    }

    std::shared_ptr<conditional_loop_node> parser::parse_conditional_loop(node_ptr parent) {
        std::shared_ptr<conditional_loop_node> cloop = std::make_shared<conditional_loop_node>(parent, *code_lexer.peek());

        if (code_lexer.peek()->get_raw_token_string() == "while") {
            code_lexer.next();

            node_ptr condition = parse_stmt(parent);

            if (condition->type != node_type::caculate && condition->type != node_type::number) {
                do_report(error_panic_code::expect, error_level::error, *code_lexer.get_last_token(), "a condition");
                return nullptr;
            }

            cloop->continue_conditions.push_back(std::move(condition));

            code_lexer.next();
            auto tok = code_lexer.get_current_token();

            if (tok->get_token_type() != token_type::colon) {
                do_report(error_panic_code::expect_after_got, error_level::error, *tok,
                    "colon", "for statement", tok->get_raw_token_string());

                return cloop;
            }

            cloop->do_block = std::move(parse_block(parent));

            code_lexer.next();

            return cloop;
        }

        code_lexer.next();

        bool end_require_parentheses = false;

        auto tok = code_lexer.peek();

        if (tok->get_raw_token_string() == "(") {
            code_lexer.next();
            end_require_parentheses = true;
        }

        while (tok && tok->get_token_type() != token_type::semicolon && tok->get_token_type() != token_type::eof) {
            node_ptr decl = std::move(parse_stmt(cloop));

            if (decl)
                cloop->init_jobs.push_back(decl);

            tok = code_lexer.peek();

            if (tok->get_token_type() == token_type::separator) {
                code_lexer.next();
                tok = code_lexer.peek();
            }
        }

        code_lexer.next();
        tok = code_lexer.peek();

        while (tok && tok->get_token_type() != token_type::semicolon && tok->get_token_type() != token_type::eof) {
            node_ptr n = std::move(parse_stmt(cloop));

            if (!n) {
                return cloop;
            }

            node_type nt = n->get_node_type();

            if (nt != node_type::caculate && nt != node_type::number) {
                do_report(error_panic_code::loop_condition_only, error_level::error, *code_lexer.get_last_token());

                return cloop;
            }

            cloop->continue_conditions.push_back(n);

            tok = code_lexer.peek();

            if (tok->get_token_type() == token_type::separator) {
                code_lexer.next();
                tok = code_lexer.peek();
            }
        }

        code_lexer.next();
        tok = code_lexer.peek();

        while (tok && tok->get_token_type() != token_type::parentheses && tok->get_token_type() != token_type::eof
            && tok->get_token_type() != token_type::colon) {
            node_ptr n = std::move(parse_stmt(cloop));
            node_type nt = n->get_node_type();

            cloop->end_jobs.push_back(n);

            tok = code_lexer.peek();

            if (tok->get_token_type() == token_type::separator) {
                code_lexer.next();
                tok = code_lexer.peek();
            }
        }

        code_lexer.next();
        tok = code_lexer.get_current_token();

        if (end_require_parentheses && tok->get_raw_token_string() != ")") {
            do_report(error_panic_code::expect_got, error_level::error, *tok,
                "close parantheses", tok->get_raw_token_string());

            return cloop;
        }

        if (end_require_parentheses && tok->get_raw_token_string() == ")") {
            code_lexer.next();
            tok = code_lexer.get_current_token();
        }

        if (tok->get_token_type() != token_type::colon) {
            do_report(error_panic_code::expect_after_got, error_level::error, *tok,
                "colon", "for statement", tok->get_raw_token_string());

            return cloop;
        }

        cloop->do_block = std::move(parse_block(cloop));

        code_lexer.next();

        return cloop;
    }

    std::shared_ptr<array_node> parser::parse_array(node_ptr parent) {
        std::shared_ptr<array_node> arr = std::make_shared<array_node>(parent, *code_lexer.peek());

        if (code_lexer.peek()->get_raw_token_string() != "(") {
            // report
            return nullptr;
        }

        code_lexer.next();
        auto tok = code_lexer.peek();

        while (tok && tok->get_raw_token_string() != ")" && tok->get_token_type() != token_type::eof) {
            node_ptr n = parse_expr(parent);

            if (n) {
                arr->init_elements.push_back(n);
            } else {
                return arr;
            }

            tok = code_lexer.peek();

            if (tok->get_raw_token_string() == ")") {
                break;
            }

            if (tok->get_token_type() != token_type::separator) {
                do_report(error_panic_code::expect_got, error_level::critical,
                    *tok, "separator", tok->get_raw_token_string());

                return arr;
            } else {
                code_lexer.next();
                tok = code_lexer.peek();
            }
        }

        return arr;
    }

    std::shared_ptr<new_object_node> parser::parse_new_object(node_ptr parent) {
        std::shared_ptr<new_object_node> obj = std::make_shared<new_object_node>(parent, *code_lexer.peek());
        code_lexer.next();

        auto tok = code_lexer.peek();

        if (tok->get_raw_token_string() == "array") {
            code_lexer.next();
            obj->object_request = std::move(parse_array(obj));

            return obj;
        }

        return nullptr;
    }

    std::shared_ptr<var_node> parser::parse_var(node_ptr parent) {
        std::shared_ptr<var_node> var = std::make_shared<var_node>(parent, *code_lexer.peek());

        // Eat var keyword
        code_lexer.next();
        code_lexer.next();

        token name = *code_lexer.get_current_token();

        if (name.get_token_type() != token_type::ident) {
            // Expected ident, got ...
            do_report(error_panic_code::expect_after, error_level::error, name,
                "var");

            return nullptr;
        }

        var->var_name = name.get_raw_token_string();

        if (get_var(parent, var->var_name)) {
            // Give error: identifier already defined
            do_report(error_panic_code::var_declared, error_level::error, name,
                name.get_raw_token_string());

            return nullptr;
        }

        // traverse node until get
        node_ptr crr = parent;

        while (crr && crr->type != node_type::function) {
            crr = crr->parent;
        }

        var->var_type = make_undefined_type();

        if (crr && crr->type == node_type::function) {
            std::dynamic_pointer_cast<function_node>(crr)->local_vars.push_back(var);
        } else {
            vars.push_back(var);
        }

        return var;
    }

    std::shared_ptr<assign_node> parser::parse_assign_node(node_ptr parent, node_ptr lhs) {
        std::shared_ptr<assign_node> assign = std::make_shared<assign_node>(parent, *code_lexer.get_current_token());
        assign->left = std::move(lhs);

        std::optional<token> tok = code_lexer.peek();
        std::string op_raw = tok->get_raw_token_string();

        code_lexer.next();

        int org_pos = code_lexer.get_pointer();

        assign->right = parse_expr(assign);

        int forward_pos = code_lexer.get_pointer();

        if (!assign->right) {
            code_lexer.jump(org_pos);
            tok = code_lexer.peek();

            if (tok->get_token_type() == token_type::number) {
                assign->right = std::move(make_number(std::stod(tok->get_raw_token_string())));
                code_lexer.next();
            } else if (tok->get_token_type() == token_type::string) {
                assign->right = std::move(make_string(tok->get_raw_token_string()));
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "none") {
                assign->right = std::make_shared<null_node>(parent, *tok);
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "(") {
                assign->right = std::move(parse_array(parent));
                code_lexer.next();
            } else if (tok->get_raw_token_string() == "new") {
                assign->right = std::move(parse_new_object(parent));
                code_lexer.next();
            } else if (tok->get_token_type() == token_type::ident) {
                assign->right = std::move(parse_ident_based(parent));
                code_lexer.next();
            } else {
                code_lexer.jump(forward_pos);

                // throw error
                do_report(error_panic_code::expect_after, error_level::error, *tok,
                    "an expression", "=");

                return nullptr;
            }
        }

        if (op_raw.length() == 2) {
            std::shared_ptr<caculate_node> fast_node = std::make_shared<caculate_node>(parent, *code_lexer.get_last_token());
            fast_node->left = assign->left;
            fast_node->right = std::move(assign->right);

            switch (op_raw[0]) {
            case '+': {
                fast_node->op = caculate_op::add;
                break;
            }
            case '-': {
                fast_node->op = caculate_op::sub;
                break;
            }
            case '*': {
                fast_node->op = caculate_op::mul;
                break;
            }
            case '/': {
                fast_node->op = caculate_op::div;
                break;
            }
            case '&': {
                fast_node->op = caculate_op::and;
                break;
            }
            case '|': {
                fast_node->op = caculate_op:: or ;
                break;
            }
            case '^': {
                fast_node->op = caculate_op:: xor ;
                break;
            }
            default: {
                assign->right = std::move(fast_node->right);
                return assign;
            }
            }

            assign->right = std::move(fast_node);
        }

        return assign;
    }

    std::shared_ptr<node> parser::parse_return(node_ptr parent) {
        std::shared_ptr<return_node> ret = std::make_shared<return_node>(parent, *code_lexer.get_current_token());

        code_lexer.next();
        ret->result = std::move(parse_rhs(ret));

        code_lexer.back();

        return ret;
    }

    std::shared_ptr<node> parser::parse_function(node_ptr parent) {
        std::shared_ptr<function_node> func = std::make_shared<function_node>(parent, *code_lexer.peek());

        code_lexer.next();
        code_lexer.next();

        std::optional<token> tok = code_lexer.get_current_token();

        if (!tok || tok->get_token_type() != token_type::ident) {
            // throw error
            do_report(error_panic_code::expect_after, error_level::error, *tok,
                "'fn'");

            return nullptr;
        }

        func->name = tok->get_raw_token_string();

        code_lexer.next();

        if (!code_lexer.get_current_token() || code_lexer.get_current_token()->get_raw_token_string() == "(") {
            code_lexer.next();

            while (code_lexer.get_current_token() && code_lexer.get_current_token()->get_raw_token_string() != ")") {
                std::string tok_str = code_lexer.get_current_token()->get_raw_token_string();

                if (tok_str != ",") {
                    var_node_ptr v = std::make_shared<var_node>(parent, *code_lexer.get_current_token());

                    v->var_name = tok_str;

                    func->local_vars.push_back(std::move(v));
                    func->args.push_back(func->local_vars.back());

                    code_lexer.next();

                    tok = code_lexer.get_current_token();

                    if (!tok || tok->get_token_type() != token_type::separator) {
                        if (!(tok->get_raw_token_string() == ")")) {
                            do_report(error_panic_code::expect_got, error_level::error, *tok,
                                "an seperator or a close paranthese", tok->get_raw_token_string());

                            return nullptr;
                        } else {
                            break;
                        }
                    }
                }

                code_lexer.next();
            }
        } else {
            code_lexer.back();
        }

        if (code_lexer.next() && code_lexer.get_current_token()->get_token_type() != token_type::colon) {
            // throw error
            do_report(error_panic_code::expect_got, error_level::error, *code_lexer.get_current_token(),
                "':'", tok->get_raw_token_string());

            return nullptr;
        }

        if (code_lexer.next() && code_lexer.get_current_token()->get_token_type() != token_type::indent) {
            // throw error
            do_report(error_panic_code::indentation_needed, error_level::error, *code_lexer.get_current_token());

            return nullptr;
        }

        auto tok_peek = code_lexer.peek();

        while (tok_peek && (tok_peek->get_token_type() != token_type::eof && tok_peek->get_token_type() != token_type::dedent)) {
            node_ptr stmt = parse_stmt(func);

            if (stmt == nullptr) {
                code_lexer.next();
                tok_peek = code_lexer.peek();

                continue;
            }

            func->add_children(stmt);
            tok_peek = code_lexer.peek();
        }

        // Emit dedent token
        code_lexer.next();

        functions.push_back(func);

        return func;
    }

    std::shared_ptr<function_call_node> parser::parse_function_call(node_ptr parent) {
        std::shared_ptr<function_call_node> func_call = std::make_shared<function_call_node>(parent, *code_lexer.get_current_token());
        std::string func_name = code_lexer.get_current_token()->get_raw_token_string();

        code_lexer.next();

        if (code_lexer.get_current_token() && code_lexer.get_current_token()->get_raw_token_string() != "(") {
            // throw err
            do_report(error_panic_code::expect_got, error_level::error, *code_lexer.get_current_token(), "(",
                code_lexer.get_current_token()->get_raw_token_string());

            return nullptr;
        }

        code_lexer.next();

        while (code_lexer.get_current_token() && code_lexer.get_current_token()->get_raw_token_string() != ")") {
            code_lexer.back();

            node_ptr arg = parse_rhs(func_call);

            auto tok = code_lexer.get_current_token();

            if (!tok || tok->get_token_type() != token_type::separator) {
                if (!(tok->get_raw_token_string() == ")")) {
                    // throw error
                    do_report(error_panic_code::expect_got, error_level::error, *tok, "a separator or close parantheses",
                        code_lexer.get_current_token()->get_raw_token_string());

                    return nullptr;
                } else {
                    func_call->args_passed.push_back(arg);

                    break;
                }
            }

            func_call->args_passed.push_back(arg);

            code_lexer.next();
        }

        func_call->func = get_function(func_name, func_call->args_passed.size());

        return func_call;
    }

    void parser::do_parsing() {
        while (code_lexer.peek() && code_lexer.peek()->get_token_type() != token_type::eof) {
            std::shared_ptr<node> node = parse_stmt(unit);

            if (node == nullptr) {
                code_lexer.next();
                continue;
            }

            unit->add_children(node);
        }
    }

    void parser::parse() {
        do_parsing();
    }
}