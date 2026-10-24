#ifndef JACK_SYMBOL_TABLE_HPP
#define JACK_SYMBOL_TABLE_HPP

#include "declarations.hpp"
#include <string>
#include <unordered_map>

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> classScope;
    std::unordered_map<std::string, Symbol> subroutineScope;
    int staticCount = 0;
    int fieldCount = 0;
    int argumentCount = 0;
    int localCount = 0;

public:
    void startClass();
    void startSubroutine();
    void define(const std::string& name, const std::string& type,
                const std::string& kind);
    int varCount(const std::string& kind) const;
    bool lookup(const std::string& name, Symbol& result) const;
    bool containsInCurrentSubroutine(const std::string& name) const;
};

#endif
