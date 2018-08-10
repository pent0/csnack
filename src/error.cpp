#include <snack/error.h>

#include <iostream>

namespace snack {
    size_t error_manager::get_total_error() {
        return actual_error;
    }

    error error_manager::make_error(error_category category, error_level level,
        int error_code, size_t column, size_t line) {
        error err;
        err.category = category;
        err.level = level;
        err.error_code = error_code;
        err.column = column;
        err.line = line;

        return err;
    }

    std::optional<std::string> error_manager::get_error_description(int error_id) {
        switch (static_cast<error_panic_code>(error_id)) {
#define DECL_ERROR(num, code, des) \
    case code:                     \
        return des;

#include <snack/error.def>
#undef DECL_ERROR

        default:
            return std::optional<std::string>{};
        }

        return std::optional<std::string>{};
    }

    error_manager::error_manager()
        : error_id_counter(0) {
    }

    bool error_manager::connect(const std::string &name, error_report_hole_ptr hole) {
        std::lock_guard guard(mut);

        if (error_dump_holes.find(name) != error_dump_holes.end()) {
            return false;
        }

        error_dump_holes.emplace(name, hole);

        return true;
    }

    bool error_manager::disconnect(const std::string &name) {
        if (error_dump_holes.find(name) == error_dump_holes.end()) {
            return false;
        }

        error_dump_holes.erase(name);

        return true;
    }

    int error_manager::throw_error(error_category place, error_level cagetory,
        int error_code, size_t column, size_t line) {
        error err = make_error(place, cagetory, error_code, column, line);
        err.description = get_error_description(error_code);

        if (cagetory == error_level::critical || cagetory == error_level::error) {
            actual_error++;
        }

        error_id_counter++;

        std::lock_guard guard(mut);
        errors.emplace(error_id_counter.load(), std::move(err));

        return error_id_counter.load();
    }

    int error_manager::throw_error(error_category place, error_level cagetory,
        int error_code, size_t column, size_t line, const std::string &template_replace1) {
        error err = make_error(place, cagetory, error_code, column, line);
        err.description = get_error_description(error_code);

        if (cagetory == error_level::critical || cagetory == error_level::error) {
            actual_error++;
        }

        if (err.description) {
            const size_t pos = err.description->find("{}");

            if (pos != std::string::npos) {
                err.description->replace(err.description->begin() + pos, err.description->begin() + pos + 2,
                    template_replace1.data());
            }
        }

        error_id_counter++;

        std::lock_guard guard(mut);
        errors.emplace(error_id_counter.load(), std::move(err));

        return error_id_counter.load();
    }

    int error_manager::throw_error(error_category place, error_level cagetory,
        int error_code, size_t column, size_t line, const std::string &template_replace1,
        const std::string &template_replace2) {
        error err = make_error(place, cagetory, error_code, column, line);
        err.description = get_error_description(error_code);

        if (cagetory == error_level::critical || cagetory == error_level::error) {
            actual_error++;
        }

        if (err.description) {
            size_t pos = err.description->find("{}");

            if (pos != std::string::npos)
                err.description->replace(err.description->begin() + pos, err.description->begin() + pos + 2,
                    template_replace1.data());

            pos = err.description->find("{}");

            if (pos != std::string::npos)
                err.description->replace(err.description->begin() + pos, err.description->begin() + pos + 2,
                    template_replace2.data());
        }

        error_id_counter++;

        std::lock_guard guard(mut);
        errors.emplace(error_id_counter.load(), std::move(err));

        return error_id_counter.load();
    }

    int error_manager::throw_error(error_category place, error_level cagetory,
        int error_code, size_t column, size_t line, std::string template_replaces[],
        int replace_count) {
        error err = make_error(place, cagetory, error_code, column, line);
        err.description = get_error_description(error_code);

        if (cagetory == error_level::critical || cagetory == error_level::error) {
            actual_error++;
        }

        if (err.description) {
            size_t pos = err.description->find("{}");

            for (int i = 0; i < replace_count - 1, pos != std::string::npos; i++) {
                err.description->replace(err.description->begin() + pos, err.description->begin() + pos + 2,
                    template_replaces[i]);

                pos = err.description->find("{}");
            }
        }

        error_id_counter++;

        std::lock_guard guard(mut);
        errors.emplace(error_id_counter.load(), std::move(err));

        return error_id_counter.load();
    }

    bool error_manager::error_out(int error_id) {
        std::lock_guard guard(mut);
        auto res = errors.find(error_id);

        if (res == errors.end()) {
            return false;
        }

        errors.erase(error_id);

        return true;
    }

    bool error_manager::dump_all_error() {
        for (auto &hole_pair : error_dump_holes) {
            for (auto &err : errors) {
                hole_pair.second->dump(err.second);
            }
        }

        errors.clear();

        return true;
    }

    class stdio_error_report_hole : public error_report_hole {
    public:
        bool open() override {
            return true;
        }

        bool dump(const error &err) override {
            switch (err.category) {
            case error_category::unit_manager:
                std::cout << "UM";
                break;

            case error_category::lexer:
                std::cout << "LX";
                break;

            case error_category::interpreter:
                std::cout << "IT";
                break;

            case error_category::parser:
                std::cout << "PR";
                break;

            default:
                return false;
            }

            std::cout << err.error_code << " @" << err.line << ":" << err.column << " => ";

            switch (err.level) {
            case error_level::critical:
                std::cout << "CRITICAL: ";
                break;

            case error_level::error:
                std::cout << "ERROR: ";
                break;

            case error_level::note:
                std::cout << "NOTE: ";
                break;

            case error_level::warn:
                std::cout << "WARN: ";
                break;

            default:
                return false;
            }

            if (err.description) {
                std::string des = *err.description;
                std::cout << des.c_str() << std::endl;
            } else {
                std::cout << "No description found yet." << std::endl;
            }

            return true;
        }

        bool close() override {
            return true;
        }
    };

    error_report_hole_ptr make_standard_stdio_hole() {
        return std::make_shared<stdio_error_report_hole>();
    }
}