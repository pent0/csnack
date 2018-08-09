#pragma once

#include <string>

namespace snack {
    enum class token_type {
        undenified,
        string,
        number,
	    hex_number,
        space,
        indent,
        dedent,
        ident,
        op,
        parentheses,
        separator,
        colon,
        eol,
        eof
    };

    class token {
        token_type type;

        std::string token_data_raw;

        size_t column;
        size_t row;

        friend class lexer;

    public:
        explicit token();

        token(const int idata);
        token(const std::string &sdata);
        token(const float fdata);

        token_type get_token_type() const;
        const std::string &get_raw_token_string() const;

        size_t get_column() const {
            return column;
        }

        size_t get_row() const {
            return row;
        }
    };
}