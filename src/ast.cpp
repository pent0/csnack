#include <snack/ast.h>

namespace snack {
    node::node()
        : parent(nullptr)
        , type(node_type::unknown) {
    }

    node::node(node_ptr parent, node_type type, token tok)
        : parent(parent)
        , type(type) {
        column = tok.get_column();
        line = tok.get_row();
    }

    node_ptr node::get_lhs() {
        return left;
    }

    node_ptr node::get_rhs() {
        return right;
    }

    node_ptr node::get_parent() {
        return parent;
    }

    caculate_node::caculate_node(node_ptr parent, token tok, caculate_op op)
        : expr_node(parent, node_type::caculate, tok)
        , op(op) {
    }

    caculate_op caculate_node::get_op() const {
        return op;
    }

    string_node::string_node(node_ptr parent, const std::string &nstr)
        : node(parent, node_type::string, token{})
        , str(nstr) {
    }

    const std::string &string_node::get_string() const {
        return str;
    }

    number_node::number_node(node_ptr parent, const long double nval)
        : node(parent, node_type::number, token{})
        , val(nval) {
    }

    const long double number_node::get_value() const {
        return val;
    }

    expr_node::expr_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::expr, tok) {
    }

    expr_node::expr_node(node_ptr parent, node_type type, token tok)
        : stmt_node(parent, type, tok) {
    }

    unit_node::unit_node(node_ptr parent)
        : stmt_node(parent, node_type::unit, token{}) {
        column = 0;
        line = 0;
    }

    stmt_node::stmt_node(node_ptr parent, token tok)
        : node(parent, node_type::stmt, tok) {
    }

    void stmt_node::add_children(node_ptr node) {
        childrens.push_back(std::move(node));
    }

    var_node::var_node(node_ptr parent, token tok)
        : node(parent, node_type::var, tok) {
    }

    var_node::var_node(node_ptr parent, token tok, const std::string &variable_name)
        : node(parent, node_type::var, tok)
        , var_name(variable_name) {
    }

    type_node::type_node(node_ptr parent)
        : node(parent, node_type::type, token{}) {
    }

    stmt_node::stmt_node(node_ptr parent, node_type type, token tok)
        : node(parent, type, tok) {
    }

    caculate_node::caculate_node(node_ptr parent, token tok)
        : expr_node(parent, node_type::caculate, tok) {
    }

    assign_node::assign_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::assign, tok) {
    }

    unary_node::unary_node(node_ptr parent, token tok)
        : expr_node(parent, node_type::unary, tok) {
    }

    function_node::function_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::function, tok) {
    }

    function_call_node::function_call_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::function_call, tok) {
    }

    return_node::return_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::ret, tok) {
    }

    null_node::null_node(node_ptr parent, token tok)
        : node(parent, node_type::null, tok) {
    }

    unit_reference_node::unit_reference_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::unit_ref, tok) {
    }

    if_else_node::if_else_node(node_ptr parent, token tok) 
        : stmt_node(parent, node_type::if_else, tok) {

    }

    block_node::block_node(node_ptr parent, token tok) 
        : stmt_node(parent, node_type::block, tok) {

    }

    conditional_loop_node::conditional_loop_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::conditional_loop, tok) {

    }

    array_node::array_node(node_ptr parent, token tok)
        : node(parent, node_type::array, tok) {

    }

    new_object_node::new_object_node(node_ptr parent, token tok)
        : stmt_node(parent, node_type::new_object, tok) {

    }

    array_access_node::array_access_node(node_ptr parent, token tok)
        : node(parent, node_type::array_access, tok) {

    }
}