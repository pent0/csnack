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

#include <snack/error.h>
#include <snack/token.h>

#include <optional>
#include <sstream>
#include <stack>
#include <vector>

namespace snack {
    class lexer {
        std::istringstream stream;
        std::istringstream global_stream;

        std::vector<token> token_list;

        size_t line;
        size_t col;

        int token_pointer;

        std::stack<size_t> indent_stack;
        std::string crr_line;

        snack::error_manager *err_mngr;

        bool ignore_indent_dendent = false;

    protected:
        token generate_indent();
        token generate_dedent();

        std::string read_util(const char stop_char);
        std::string read_util(const char stop_char1, const char stop_char2);
        std::string read_util(const char stop_char1, const char stop_char2, const char stop_char3);

        std::string read_while(const char stop_char);

        void generate_indent_dedent();
        void lex_line();

        void lex_string();
        void lex_number();
        void lex_ident();
        void lex_operator();

        void do_report(int error_code, error_level level, token tok);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1);
        void do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1,
            const std::string &arg2);

    public:
        explicit lexer();

        lexer(snack::error_manager &mngr, std::istringstream &source_stream);

        std::optional<token> get_current_token();
        std::optional<token> get_last_token();

        bool next();
        bool back();

        void jump(int ptr);
        int get_pointer() const;

        std::optional<token> peek();
    };
}