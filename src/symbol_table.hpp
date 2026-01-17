#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

enum class SymbolType : uint8_t { VARIABLE, FUNCTION };

struct SymbolEntry {
    std::string name;
    SymbolType type;
    uint64_t address;                // Memory addr (variable) or code addr (function)
    std::vector<std::string> params; // For functions only

    bool is_variable() const {
        return type == SymbolType::VARIABLE;
    }
    bool is_function() const {
        return type == SymbolType::FUNCTION;
    }
};

class SymbolTable {
  private:
    std::map<std::string, SymbolEntry> symbols;
    std::vector<std::string> insertion_order; // For ordered iteration

  public:
    // Define a variable symbol
    void define_variable(const std::string& name, uint64_t address) {
        // Check if already exists (update address if so)
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            it->second.address = address;
            return;
        }

        SymbolEntry entry;
        entry.name = name;
        entry.type = SymbolType::VARIABLE;
        entry.address = address;

        symbols[name] = entry;
        insertion_order.push_back(name);
    }

    // Define a function symbol
    void define_function(const std::string& name, uint64_t code_address,
                         const std::vector<std::string>& params) {
        // Check if already exists (update if so)
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            it->second.address = code_address;
            it->second.params = params;
            return;
        }

        SymbolEntry entry;
        entry.name = name;
        entry.type = SymbolType::FUNCTION;
        entry.address = code_address;
        entry.params = params;

        symbols[name] = entry;
        insertion_order.push_back(name);
    }

    // Look up a symbol by name
    std::optional<SymbolEntry> lookup(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Check if a symbol exists
    bool exists(const std::string& name) const {
        return symbols.find(name) != symbols.end();
    }

    // Get all variable symbols (in insertion order)
    std::vector<SymbolEntry> all_variables() const {
        std::vector<SymbolEntry> result;
        for (const auto& name : insertion_order) {
            auto it = symbols.find(name);
            if (it != symbols.end() && it->second.is_variable()) {
                result.push_back(it->second);
            }
        }
        return result;
    }

    // Get all function symbols (in insertion order)
    std::vector<SymbolEntry> all_functions() const {
        std::vector<SymbolEntry> result;
        for (const auto& name : insertion_order) {
            auto it = symbols.find(name);
            if (it != symbols.end() && it->second.is_function()) {
                result.push_back(it->second);
            }
        }
        return result;
    }

    // Get all symbols (in insertion order)
    std::vector<SymbolEntry> all_symbols() const {
        std::vector<SymbolEntry> result;
        for (const auto& name : insertion_order) {
            auto it = symbols.find(name);
            if (it != symbols.end()) {
                result.push_back(it->second);
            }
        }
        return result;
    }

    // Merge another symbol table into this one (other's symbols override on collision)
    void merge(const SymbolTable& other) {
        for (const auto& name : other.insertion_order) {
            auto it = other.symbols.find(name);
            if (it != other.symbols.end()) {
                const auto& entry = it->second;
                if (entry.is_variable()) {
                    define_variable(entry.name, entry.address);
                } else {
                    define_function(entry.name, entry.address, entry.params);
                }
            }
        }
    }

    // Clear all symbols
    void clear() {
        symbols.clear();
        insertion_order.clear();
    }

    // Get number of symbols
    size_t size() const {
        return symbols.size();
    }

    // Check if empty
    bool empty() const {
        return symbols.empty();
    }

    // Get count of variables
    size_t variable_count() const {
        size_t count = 0;
        for (const auto& [name, entry] : symbols) {
            if (entry.is_variable())
                count++;
        }
        return count;
    }

    // Get count of functions
    size_t function_count() const {
        size_t count = 0;
        for (const auto& [name, entry] : symbols) {
            if (entry.is_function())
                count++;
        }
        return count;
    }
};
