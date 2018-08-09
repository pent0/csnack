#include <snack/unit/init.h>
#include <snack/unit/std.h>

#include <snack/unit_manager.h>

namespace snack::userspace {
    void init_builtin(unit_manager *umngr) {
        #define ADD_UNIT(unit) umngr->add_external_unit(std::make_shared<##unit>())

        ADD_UNIT(std_unit);

        #undef ADD_UNIT
    }
}