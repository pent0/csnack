#include <snack/unit/init.h>
#include <snack/unit_manager.h>

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace snack::userspace {
    unit_manager::unit_manager(snack::error_manager &mngr, const bool import_default_external)
        : err_mngr(&mngr) {
        if (import_default_external) {
            init_builtin(this);
        }

        search_paths.push_back(".");
    }

    bool unit_manager::do_unit_check(const std::string &unit_name, FILE *f) {
        if (!f) {
            return false;
        }

        char magic_check[4];

        fread(magic_check, 1, 4, f);

        if (magic_check[0] == 'S' && magic_check[1] == 'N' && magic_check[2] == 'L' && magic_check[3] == '\0') {
            // Considering a script is small, really. No script will be 4 MB ;), unless you fill garbage
            fseek(f, 0, SEEK_END);
            size_t fsize = ftell(f);

            fseek(f, 0, SEEK_SET);

            unit_buffer_map[unit_name].resize(fsize);

            fread(&(unit_buffer_map[unit_name][0]), 1, fsize, f);
            units.emplace(unit_name, std::make_shared<interpreted_unit>(unit_name, unit_buffer_map[unit_name].data()));
        }

        return true;
    }

    unit_ptr unit_manager::use_unit(const std::string &unit) {
        if (units.find(unit) != units.end()) {
            return units[unit];
        }

        for (const auto &path : search_paths) {
            fs::directory_iterator dir(path);

            for (const auto &entry : dir) {
                if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".snc" && entry.path().filename() == unit) {
                    FILE *f = fopen(entry.path().string().c_str(), "rb");

                    if (do_unit_check(unit, f)) {
                        fclose(f);
                        return units[unit];
                    }

                    fclose(f);
                }
            }
        }

        do_report(error_panic_code::unit_not_found, error_level::critical, unit);

        return nullptr;
    }

    void unit_manager::add_external_unit(unit_ptr unit) {
        units.emplace(unit->get_unit_name(), std::move(unit));
    }

    void unit_manager::add_search_path(const std::string &path) {
        search_paths.push_back(path);
    }

    void unit_manager::do_report(error_panic_code error_code, error_level level) {
        if (err_mngr) {
            err_mngr->throw_error(error_category::unit_manager, level,
                static_cast<int>(error_code), 0, 0);
        }
    }

    void unit_manager::do_report(error_panic_code error_code, error_level level, const std::string &arg0) {
        err_mngr->throw_error(error_category::unit_manager, level,
            static_cast<int>(error_code), 0, 0, arg0);
    }
}