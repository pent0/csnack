#include <snack/ir_compiler.h>
#include <snack/ir_decompiler.h>
#include <snack/ir_interpreter.h>

#include <snack/error.h>

#include <snack/lexer.h>
#include <snack/parser.h>

#include <iostream>

const char *test_script = {
    "uses std\n"
    "\n"
    "fn max(a, b):\n"
    "    if a > b:\n"
    "        ret a\n"
    "\n"
    "    ret b\n"
    "\n"
    "fn test:\n"
    "    var a = 6\n"
    "    var b = 7 + max(5, 6)\n"
    "    var c = new array(12, 15, 27)\n"
    "\n"
    "    var d = 12\n"
    "    for var i = 0; i < length(c); i+=1:\n"
    "       var d = 7\n"
    "       print(\"{} {} \", c[i], d)\n"
    "\n"
    "    for var i = 0; i < length(c); i+=1:\n"
    "       print('{} ', i)\n"
    "\n"
    "fn main:\n"
    "    test()\n"
    "\n"
};

void error_manager_test() {
    snack::error_manager err_mngr{};
    err_mngr.connect("stdio hole", snack::make_standard_stdio_hole());

    err_mngr.throw_error(snack::error_category::unit_manager, snack::error_level::critical,
        static_cast<int>(snack::error_panic_code::unit_not_found),
        0, 0, "your mom");

    err_mngr.dump_all_error();
}

void compiler_test() {
    std::cout << "Original script: " << std::endl;
    std::cout << std::endl;
    std::cout << test_script << std::endl;
    std::cout << std::endl;

    std::cout << "IR emitted: " << std::endl;
    std::cout << std::endl;

    std::istringstream stream;
    stream.str(test_script);

    snack::error_manager err_mngr{};
    err_mngr.connect("stdio hole", snack::make_standard_stdio_hole());

    snack::lexer lexer(err_mngr, stream);
    snack::parser parser(err_mngr, lexer);

    parser.parse();

    if (err_mngr.get_total_error()) {
        err_mngr.dump_all_error();
        return;
    }

    snack::userspace::unit_manager manager(err_mngr);

    snack::ir::backend::ir_compiler compiler(err_mngr, manager);
    compiler.compile(parser.get_unit_node());

    if (err_mngr.get_total_error()) {
        err_mngr.dump_all_error();
        return;
    }

    std::string res = compiler.get_compile_binary();
    size_t start = compiler.get_code_start();

    std::istringstream destream(res);

    snack::ir::frontend::ir_decompiler decompiler(err_mngr);
    decompiler.supply(destream);

    decompiler.dump();

    std::cout << std::endl;

    if (err_mngr.get_total_error()) {
        err_mngr.dump_all_error();
        return;
    }

    std::cout << "Used unit";
    std::cout << std::endl;

    decompiler.dump_unit_ref_table();

    std::cout << std::endl;

    snack::ir::backend::ir_interpreter interpreter(err_mngr, manager);
    manager.add_external_unit(std::make_shared<snack::userspace::interpreted_unit>("bim", res.data()));

    snack::userspace::unit_ptr unit = manager.use_unit("bim");
    size_t index = *unit->get_function_idx("main", 0);

    unit->call_function(&interpreter, index, nullptr);

    std::cout << "Interpreted result:";
    std::cout << std::endl;

    interpreter.interpret();
}

int main() {
    compiler_test();

    std::getchar();
    return 0;
}