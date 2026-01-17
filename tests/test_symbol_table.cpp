#include "../src/symbol_table.hpp"
#include <cassert>
#include <iostream>

void test_define_variable() {
    std::cout << "Testing define_variable..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    assert(table.exists("x"));
    assert(table.size() == 1);

    auto entry = table.lookup("x");
    assert(entry.has_value());
    assert(entry->name == "x");
    assert(entry->type == SymbolType::VARIABLE);
    assert(entry->address == 1000);
    assert(entry->is_variable());
    assert(!entry->is_function());

    std::cout << "  ✓ define_variable works correctly" << '\n';
}

void test_define_function() {
    std::cout << "Testing define_function..." << '\n';

    SymbolTable table;

    std::vector<std::string> params = {"a", "b"};
    table.define_function("add", 2000, params);

    assert(table.exists("add"));
    assert(table.size() == 1);

    auto entry = table.lookup("add");
    assert(entry.has_value());
    assert(entry->name == "add");
    assert(entry->type == SymbolType::FUNCTION);
    assert(entry->address == 2000);
    assert(entry->params.size() == 2);
    assert(entry->params[0] == "a");
    assert(entry->params[1] == "b");
    assert(entry->is_function());
    assert(!entry->is_variable());

    std::cout << "  ✓ define_function works correctly" << '\n';
}

void test_lookup_nonexistent() {
    std::cout << "Testing lookup of nonexistent symbol..." << '\n';

    SymbolTable table;

    assert(!table.exists("foo"));
    auto entry = table.lookup("foo");
    assert(!entry.has_value());

    std::cout << "  ✓ lookup returns nullopt for nonexistent symbols" << '\n';
}

void test_multiple_symbols() {
    std::cout << "Testing multiple symbols..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    table.define_variable("y", 1001);
    table.define_function("square", 500, {"n"});
    table.define_variable("z", 1002);
    table.define_function("add", 600, {"a", "b"});

    assert(table.size() == 5);
    assert(table.variable_count() == 3);
    assert(table.function_count() == 2);

    // Verify all symbols exist
    assert(table.exists("x"));
    assert(table.exists("y"));
    assert(table.exists("z"));
    assert(table.exists("square"));
    assert(table.exists("add"));

    std::cout << "  ✓ Multiple symbols defined correctly" << '\n';
}

void test_all_variables() {
    std::cout << "Testing all_variables..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    table.define_function("f", 500, {});
    table.define_variable("y", 1001);

    auto vars = table.all_variables();
    assert(vars.size() == 2);
    // Should be in insertion order
    assert(vars[0].name == "x");
    assert(vars[1].name == "y");

    std::cout << "  ✓ all_variables returns variables in order" << '\n';
}

void test_all_functions() {
    std::cout << "Testing all_functions..." << '\n';

    SymbolTable table;

    table.define_function("f", 500, {});
    table.define_variable("x", 1000);
    table.define_function("g", 600, {"a"});

    auto funcs = table.all_functions();
    assert(funcs.size() == 2);
    // Should be in insertion order
    assert(funcs[0].name == "f");
    assert(funcs[1].name == "g");

    std::cout << "  ✓ all_functions returns functions in order" << '\n';
}

void test_all_symbols() {
    std::cout << "Testing all_symbols..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    table.define_function("f", 500, {});
    table.define_variable("y", 1001);

    auto syms = table.all_symbols();
    assert(syms.size() == 3);
    // Should be in insertion order
    assert(syms[0].name == "x");
    assert(syms[1].name == "f");
    assert(syms[2].name == "y");

    std::cout << "  ✓ all_symbols returns all symbols in order" << '\n';
}

void test_update_existing() {
    std::cout << "Testing update of existing symbol..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    assert(table.lookup("x")->address == 1000);

    // Redefine with new address
    table.define_variable("x", 2000);
    assert(table.size() == 1);                  // Still only 1 symbol
    assert(table.lookup("x")->address == 2000); // Updated address

    std::cout << "  ✓ Redefining symbol updates address" << '\n';
}

void test_merge() {
    std::cout << "Testing merge..." << '\n';

    SymbolTable table1;
    table1.define_variable("x", 1000);
    table1.define_function("f", 500, {});

    SymbolTable table2;
    table2.define_variable("y", 1001);
    table2.define_variable("x", 2000); // Override x

    table1.merge(table2);

    assert(table1.size() == 3);                  // x, f, y
    assert(table1.lookup("x")->address == 2000); // Overridden
    assert(table1.lookup("y")->address == 1001);
    assert(table1.lookup("f")->address == 500);

    std::cout << "  ✓ merge combines tables correctly" << '\n';
}

void test_clear() {
    std::cout << "Testing clear..." << '\n';

    SymbolTable table;

    table.define_variable("x", 1000);
    table.define_function("f", 500, {});
    assert(!table.empty());

    table.clear();

    assert(table.empty());
    assert(table.size() == 0);
    assert(!table.exists("x"));
    assert(!table.exists("f"));

    std::cout << "  ✓ clear removes all symbols" << '\n';
}

void test_empty_table() {
    std::cout << "Testing empty table..." << '\n';

    SymbolTable table;

    assert(table.empty());
    assert(table.size() == 0);
    assert(table.variable_count() == 0);
    assert(table.function_count() == 0);
    assert(table.all_symbols().empty());
    assert(table.all_variables().empty());
    assert(table.all_functions().empty());

    std::cout << "  ✓ Empty table behaves correctly" << '\n';
}

void test_function_with_no_params() {
    std::cout << "Testing function with no parameters..." << '\n';

    SymbolTable table;

    table.define_function("noop", 100, {});

    auto entry = table.lookup("noop");
    assert(entry.has_value());
    assert(entry->params.empty());

    std::cout << "  ✓ Function with no params works" << '\n';
}

int main() {
    std::cout << "=== Symbol Table Unit Tests ===" << '\n' << '\n';

    test_define_variable();
    test_define_function();
    test_lookup_nonexistent();
    test_multiple_symbols();
    test_all_variables();
    test_all_functions();
    test_all_symbols();
    test_update_existing();
    test_merge();
    test_clear();
    test_empty_table();
    test_function_with_no_params();

    std::cout << '\n' << "=== All Symbol Table Tests Passed! ===" << '\n';
    return 0;
}
