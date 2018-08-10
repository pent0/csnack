#include <snack/lexer.h>

#include <map>
#include <regex>

namespace snack {
    std::map<std::string, token_type> patterns = {
        { "['\"][^']*[']", token_type::string },
        { "[A-Za-z][A-Za-z0-9_.]*", token_type::ident },
        { "[><*=&|-]?[*+\\-=><&>=\\|\\^<]", token_type::op },
        { "([0-9]*[.])?[0-9]+", token_type::number },
        { "[$][0-9a-fA-F]+", token_type::hex_number },
        { "[\\(\\)]", token_type::parentheses },
        { ",", token_type::separator },
        { ":", token_type::colon }
    };

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

    token lexer::generate_ident() {
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

    void lexer::generate_ident_dedent() {
        size_t i = 0;

        while (crr_line[i] == ' ')
            i++;

        while (crr_line.length() > 0 && crr_line[i] == '#') {
            std::getline(stream, crr_line);

            line++;
            col = 1;

            i = 0;
            
            while (crr_line[i] == ' ')
                i++;
        }

        size_t indent_level = i;

        if (indent_level > indent_stack.top()) {
            token_list.push_back(std::move(generate_ident()));
            indent_stack.push(indent_level);
        } else if (indent_level < indent_stack.top()) {
            while (indent_level < indent_stack.top()) {
                token_list.push_back(std::move(generate_dedent()));
                indent_stack.pop();
            }
        }

        col += i;
    }

    void lexer::lex_line() {
        line += 1;
        col = 1;

        generate_ident_dedent();

        std::map<size_t, std::pair<std::string, token_type>> matches;

        for (const auto &pattern : patterns) {
            std::regex r(pattern.first);

            auto words_begin = std::sregex_iterator(crr_line.begin(), crr_line.end(), r);
            auto words_end = std::sregex_iterator();

            // No look behind so this is painful
            std::string last_string = "";

            for (auto it = words_begin; it != words_end; ++it) {
                std::string raw = it->str();
                token_type type = pattern.second;

                if (pattern.second == token_type::string) {
                    raw = raw.substr(1, raw.length() - 2);
                } else if (pattern.second == token_type::hex_number) {
                    raw = raw.substr(1, raw.length() - 1);
                    raw = std::to_string(std::stoul(raw, nullptr, 16));

                    type = token_type::number;
                } 

                matches[it->position()] = make_pair(raw, type);
            }
        }

        // Filter all mismatch identifiers
        std::regex r("['\"][^']*[']");

        auto words_begin = std::sregex_iterator(crr_line.begin(), crr_line.end(), r);
        auto words_end = std::sregex_iterator();

        for (auto it = words_begin; it != words_end; it++) {
            auto res = std::find_if(matches.begin(), matches.end(),
                [&](auto pair) { return (pair.second.second == token_type::ident) && 
                (it->position() <= pair.first) && (pair.first < it->position() + it->str().length()); });

            while (res != matches.end()) {
                matches.erase(res);
                res = std::find_if(matches.begin(), matches.end(),
                    [&](auto pair) { return (pair.second.second == token_type::ident) &&
                    (it->position() <= pair.first) && (pair.first < it->position() + it->str().length()); });
            }
        }

        for (const auto &match : matches) {
            token new_token;

            new_token.column = match.first + 1;
            new_token.row = line;
            new_token.type = match.second.second;
            new_token.token_data_raw = match.second.first;

            token_list.push_back(new_token);
        }
    }

    lexer::lexer()
        : line(0)
        , col(1)
        , token_pointer(-1) {
        indent_stack.push(0);
    }

    lexer::lexer(snack::error_manager &mngr, std::istringstream &source_stream)
        : stream(std::move(source_stream))
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
            std::getline(stream, crr_line);

            lex_line();

            if (stream.eof()) {
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