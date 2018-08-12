#include <snack/lexer.h>

#include <map>
#include <regex>

namespace snack {
    void lexer::do_report(int error_code, error_level level, token tok) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::lexer, level, error_code,
                tok.get_column(), tok.get_row());
        }
    }

    void lexer::do_report(int error_code, error_level level, token tok, const std::string &arg0) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::lexer, level, error_code,
                tok.get_column(), tok.get_row(), arg0);
        }
    }

    void lexer::do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1,
        const std::string &arg2) {
        if (err_mngr) {
            std::string temp_arr[] = { arg0, arg1, arg2 };

            err_mngr->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row(), temp_arr, 3);
        }
    }

    void lexer::do_report(int error_code, error_level level, token tok, const std::string &arg0, const std::string &arg1) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::parser, level, error_code,
                tok.get_column(), tok.get_row(), arg0, arg1);
        }
    }

    std::string lexer::read_util(const char stop_char) {
        std::string result;
        char single;

        while (stream.peek() != stop_char) {
            stream.read(&single, 1);
            result += single;
        }

        return result;
    }

    std::string lexer::read_util(const char stop_char1, const char stop_char2) {
        std::string result;
        char single;

        while (stream.peek() != stop_char1 && stream.peek() != stop_char2) {
            stream.read(&single, 1);
            result += single;
        }

        return result;
    }

    std::string lexer::read_util(const char stop_char1, const char stop_char2, const char stop_char3) {
        std::string result;
        char single;

        while (stream.peek() != stop_char1 && stream.peek() != stop_char2 && stream.peek() != stop_char3) {
            stream.read(&single, 1);
            result += single;
        }

        return result;
    }

    std::string lexer::read_while(const char stop_char) {
        std::string result;
        char single;

        while (stream.peek() == stop_char) {
            stream.read(&single, 1);
            result += single;
        }

        return result;
    }

    token lexer::generate_indent() {
        token new_token;

        new_token.column = 1;
        new_token.row = line;

        new_token.token_data_raw = "indent";
        new_token.type = token_type::indent;

        return new_token;
    }

    token lexer::generate_dedent() {
        token new_token;

        new_token.column = 1;
        new_token.row = line;

        new_token.token_data_raw = "dedent";
        new_token.type = token_type::dedent;

        return new_token;
    }

    void lexer::generate_indent_dedent() {
        size_t i = 0;

        while (crr_line[i] == ' ')
            i++;

        size_t indent_level = i;

        if (indent_level > indent_stack.top()) {
            token_list.push_back(std::move(generate_indent()));
            indent_stack.push(indent_level);
        } else if (indent_level < indent_stack.top()) {
            while (indent_level < indent_stack.top()) {
                token_list.push_back(std::move(generate_dedent()));
                indent_stack.pop();
            }
        }

        col += i;
    }

    void lexer::lex_string() {
        token tok;
        tok.type = token_type::string;
        tok.column = stream.tellg();
        tok.row = line;

        std::string raw_token_string = "";
        char capture_seq;
        stream.read(&capture_seq, 1);

        raw_token_string += read_util(capture_seq);
        col += raw_token_string.length() + 2;

        char temp;
        stream.read(&temp, 1);

        if (raw_token_string.size() >= 3) {
            while (raw_token_string[raw_token_string.length() - 2] == '\\') {
                raw_token_string += read_util(capture_seq);
                col += raw_token_string.length() + 2;

                char temp;
                stream.read(&temp, 1);
            }
        }

        auto pos = raw_token_string.find_first_of('\\');

        while (pos != std::string::npos) {
            if (raw_token_string[pos - 1] != '\\' || (raw_token_string.length() > pos - 1 && raw_token_string[pos + 1] != '\\')) {
                // Missing closing quote lexical warning
                do_report(error_panic_code::missing_closing_quote, error_level::warn, tok);
            }

            raw_token_string.erase(pos);
        }

        tok.token_data_raw = raw_token_string;

        token_list.push_back(tok);
    }

    void lexer::lex_number() {
        token tok;
        tok.type = token_type::number;
        tok.column = col + 1;
        tok.row = line;

        bool hex = (stream.peek() == '$') ? true : false;
        bool dot_found = false;

        if (hex) {
            tok.type = token_type::hex_number;
        }

        std::string raw_num_string;

        if (hex) {
            char temp;
            stream.read(&temp, 1);

            temp = stream.peek();
            while ((temp >= '0' && temp <= '9') || (temp >= 'A' && temp <= 'F') || (temp >= 'a' && temp <= 'f')
                || temp == '.') {
                stream.read(&temp, 1);

                if (temp == '.' && dot_found) {
                    // error
                    do_report(error_panic_code::invalid_number_dot, error_level::error,
                        tok);
                } else {
                    dot_found = true;
                }

                raw_num_string += temp;
                temp = stream.peek();
            }

            col += raw_num_string.length() + 1;
        } else {
            char temp = stream.peek();

            while (temp == '.' || (temp >= '0' && temp <= '9')) {
                stream.read(&temp, 1);

                if (temp == '.' && dot_found) {
                    // error
                    do_report(error_panic_code::invalid_number_dot, error_level::error,
                        tok);
                } else {
                    dot_found = true;
                }

                raw_num_string += temp;
                temp = stream.peek();
            }

            col += raw_num_string.length();
        }

        tok.token_data_raw = raw_num_string;

        token_list.push_back(tok);
    }

    void lexer::lex_operator() {
        token tok;
        tok.column = col + 1;
        tok.row = line;
        tok.type = token_type::op;

        char temp = stream.peek();
        std::string op;

        while (temp == '+' || temp == '-' || temp == '*' || temp == '//' || temp == '&' || temp == '|'
            || temp == '^' || temp == '>' || temp == '=' || temp == '<' || temp == '~' || temp == '!') {
            stream.read(&temp, 1);

            op += temp;
            temp = stream.peek();
        }

        tok.token_data_raw = op;
        col += op.length();

        token_list.push_back(tok);
    }

    std::vector<std::string> keywords = {
        "new",
        "var",
        "array",
        "object",
        "fn",
        "ret",
        "for",
        "while",
        "do",
        "unless",
        "uses",
        "if"
    };

    void lexer::lex_ident() {
        token tok;

        tok.column = col + 1;
        tok.row = line;
        tok.type = token_type::ident;

        char c = stream.peek();
        std::string ident;

        while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '$') {
            stream.read(&c, 1);

            ident += c;
            c = stream.peek();
        }

        tok.token_data_raw = ident;
        col += ident.length();

        auto keyword = std::find(keywords.begin(), keywords.end(), ident);
        if (keyword != keywords.end()) {
            tok.type = token_type::keyword;
        }

        token_list.push_back(tok);
    }

    void lexer::lex_line() {
        col = 1;

        stream = std::istringstream();
        stream.str(crr_line);

        if (!ignore_indent_dendent) {
            generate_indent_dedent();
        }

        stream.seekg(col - 1);

        while (!stream.eof()) {
            switch (stream.peek()) {
            case '\'':
            case '"': {
                lex_string();
                break;
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '$': {
                lex_number();
                break;
            }

            case '(':
            case ')': {
                token tok;
                tok.type = token_type::parentheses;
                tok.column = col;
                tok.row = line;

                char eat = 0;
                stream.read(&eat, 1);

                tok.token_data_raw = eat;

                if (eat == '(') {
                    ignore_indent_dendent = true;
                } else {
                    ignore_indent_dendent = false;
                }

                col += 1;

                token_list.push_back(tok);

                break;
            }

            case ' ': {
                std::string space = read_while(stream.peek());
                col += space.length();

                break;
            }

            case '\t':
            case '\n': {
                char temp;
                stream.read(&temp, 1);

                col += 1;
                break;
            }

            case '[':
            case ']': {
                token tok;
                tok.type = token_type::square_bracket;
                tok.column = col;
                tok.row = line;

                char eat = 0;
                stream.read(&eat, 1);

                tok.token_data_raw = eat;

                token_list.push_back(tok);
                break;
            }

            case '-':
            case '+':
            case '*':
            case '//':
            case '&':
            case '^':
            case '|':
            case '=':
            case '>':
            case '<':
            case '~':
            case '!': {
                lex_operator();
                break;
            }

            case ',': {
                token tok;
                tok.type = token_type::separator;
                tok.column = col;
                tok.row = line;

                char eat = 0;
                stream.read(&eat, 1);

                tok.token_data_raw = eat;

                token_list.push_back(tok);

                break;
            }

            case ';': {
                token tok;
                tok.type = token_type::semicolon;
                tok.column = col;
                tok.row = line;

                char eat = 0;
                stream.read(&eat, 1);

                tok.token_data_raw = eat;

                token_list.push_back(tok);

                break;
            }

            case ':': {
                token tok;
                tok.type = token_type::colon;
                tok.column = col;
                tok.row = line;

                char eat = 0;
                stream.read(&eat, 1);

                tok.token_data_raw = eat;

                token_list.push_back(tok);

                break;
            }

            default: {
                auto c = stream.peek();

                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
                    lex_ident();
                    break;
                }

                if (c != -1) {
                    token tok;
                    tok.type = token_type::colon;
                    tok.column = col;
                    tok.row = line;

                    do_report(error_panic_code::unrecognize_token, error_level::critical, tok);

                    char temp;
                    stream.read(&temp, 1);

                }

                break;
            }
            }
        }
    }

    lexer::lexer()
        : line(0)
        , col(1)
        , token_pointer(-1) {
        indent_stack.push(0);
    }

    lexer::lexer(snack::error_manager &mngr, std::istringstream &source_stream)
        : global_stream(std::move(source_stream))
        , line(0)
        , col(1)
        , token_pointer(-1)
        , err_mngr(&mngr) {
        indent_stack.push(0);
    }

    std::optional<token> lexer::get_current_token() {
        if (token_list.size() == 0) {
            return std::optional<token>{};
        }

        return token_list[token_pointer];
    }

    std::optional<token> lexer::get_last_token() {
        if (token_list.size() < 2 || token_pointer == 0) {
            return std::optional<token>{};
        }

        return token_list[token_pointer - 1];
    }

    bool lexer::next() {
        if (token_list.size() > 0 && token_pointer >= 0 && token_list[token_pointer].type == token_type::eof) {
            return false;
        }

        token_pointer++;

        if (token_pointer == 0 || token_pointer >= token_list.size() - 1) {
            line += 1;

            std::getline(global_stream, crr_line);

            while (!global_stream.eof() && (crr_line.length() == 0 || crr_line.find_first_not_of("\n\t ") == std::string::npos)) {
                line += 1;
                std::getline(global_stream, crr_line);
            }

            if (crr_line.length() > 0)
                lex_line();

            if (global_stream.eof()) {
                token new_token;

                new_token.row = line + 1;
                new_token.column = 1;
                new_token.type = token_type::eof;
                new_token.token_data_raw = "EOF";

                token_list.push_back(new_token);
            }
        }

        return true;
    }

    bool lexer::back() {
        if (token_pointer - 1 < -1) {
            return false;
        }

        token_pointer--;

        return true;
    }

    std::optional<token> lexer::peek() {
        next();
        std::optional<token> ret = get_current_token();
        token_pointer--;

        return ret;
    }

    void lexer::jump(int ptr) {
        token_pointer = ptr;
    }

    int lexer::get_pointer() const {
        return token_pointer;
    }
}