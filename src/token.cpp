#include <snack/token.h>

namespace snack {
    token::token()
        : type(token_type::undenified) {
    }

    token::token(const int idata)
        : type(token_type::number) {
        token_data_raw = std::to_string(idata);
    }

    token::token(const std::string &sdata)
        : type(token_type::string) {
        token_data_raw = std::move(sdata);
    }

    token::token(const float fdata)
        : type(token_type::number) {
        token_data_raw = std::to_string(fdata);
    }

    token_type token::get_token_type() const {
        return type;
    }

    const std::string &token::get_raw_token_string() const {
        return token_data_raw;
    }
}