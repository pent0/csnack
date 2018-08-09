set (SNACK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(snack 
    ${SNACK_INCLUDE_DIR}/snack/lexer.h
    ${SNACK_INCLUDE_DIR}/snack/unit.h
    ${SNACK_INCLUDE_DIR}/snack/ast.h
    ${SNACK_INCLUDE_DIR}/snack/token.h
    ${SNACK_INCLUDE_DIR}/snack/error.h
    ${SNACK_INCLUDE_DIR}/snack/parser.h
    ${SNACK_INCLUDE_DIR}/snack/unit.h
    ${SNACK_INCLUDE_DIR}/snack/unit_manager.h
    ${SNACK_INCLUDE_DIR}/snack/ir_interpreter.h
    ${SNACK_INCLUDE_DIR}/snack/ir_decompiler.h
    ${SNACK_INCLUDE_DIR}/snack/ir_compiler.h
    ${SNACK_INCLUDE_DIR}/snack/ir_opcode.h
    ${SNACK_INCLUDE_DIR}/snack/unit/std.h
    ${SNACK_INCLUDE_DIR}/snack/unit/init.h
    src/token.cpp
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/error.cpp
    src/unit.cpp
    src/unit_manager.cpp
    src/ir_compiler.cpp
    src/ir_decompiler.cpp
    src/ir_interpreter.cpp
    src/ir_opcode.cpp
    src/unit/std.cpp
    src/unit/init.cpp)

target_include_directories(snack PUBLIC ${SNACK_INCLUDE_DIR})

add_executable(comdisint example/comdisint.cpp)
target_link_libraries(comdisint PRIVATE snack)