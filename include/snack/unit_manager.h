#pragma once

#include <snack/error.h>
#include <snack/unit.h>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace snack::ir::backend {
    class ir_interpreter;
}

namespace snack::userspace {
    class unit_manager {
        std::unordered_map<std::string, unit_ptr> units;
        std::vector<std::string> search_paths;

        std::unordered_map<std::string, std::vector<char>> unit_buffer_map;

        snack::error_manager *err_mngr;

    protected:
        bool do_unit_check(const std::string &unit, FILE *f);

        void do_report(error_panic_code error_code, error_level level);
        void do_report(error_panic_code error_code, error_level level, const std::string &arg0);

    public:
        explicit unit_manager(snack::error_manager &mngr, const bool import_default_external = true);

        unit_ptr use_unit(const std::string &unit);

        void add_external_unit(unit_ptr unit);
        void add_search_path(const std::string &path);
    };
}