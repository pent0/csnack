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

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace snack {
    enum error_panic_code {
        #define DECL_ERROR(a, b, c) b = a,
         #include <snack/error.def>
        #undef DECL_ERROR
        total_error
    };

    enum class error_level {
        note,
        warn,
        error,
        critical
    };

    enum class error_category {
        lexer,
        parser,
        interpreter,
        unit_manager,
        compiler
    };

    struct error {
        error_category category;
        error_level level;

        int error_code;

        size_t column;
        size_t line;

        std::optional<std::string> description;
    };

    /*! \brief A hole for error manager to dump all errors to. */
    class error_report_hole {
    public:
        virtual bool open() = 0;
        virtual bool dump(const error &err) = 0;

        virtual bool close() = 0;
    };

    using error_report_hole_ptr = std::shared_ptr<error_report_hole>;

    error_report_hole_ptr make_standard_stdio_hole();

    class error_manager {
        std::map<std::string, error_report_hole_ptr> error_dump_holes;

        std::map<int, error> errors;
        std::atomic<int> error_id_counter;

        size_t actual_error = 0;

        std::mutex mut;

    protected:
        std::optional<std::string> get_error_description(int error_id);

        error make_error(error_category category, error_level level,
            int error_code, size_t column, size_t line);

    public:
        explicit error_manager();

        bool connect(const std::string &name, error_report_hole_ptr hole);
        bool disconnect(const std::string &name);

        int throw_error(error_category category, error_level level,
            int error_code, size_t column, size_t line);

        int throw_error(error_category category, error_level level,
            int error_code, size_t column, size_t line, const std::string &template_replace1);

        int throw_error(error_category category, error_level level,
            int error_code, size_t column, size_t line, const std::string &template_replace1,
            const std::string &template_replace2);

        int throw_error(error_category category, error_level level,
            int error_code, size_t column, size_t line, std::string template_replaces[],
            int replace_count);

        bool error_out(int error_id);
        bool dump_all_error();

        size_t get_total_error();
    };
}