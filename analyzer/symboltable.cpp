#include "symboltable.hpp"
#include <stdexcept>

void SymbolTable::startClass() {
    classScope.clear();
    subroutineScope.clear();
    staticCount = fieldCount = argumentCount = localCount = 0;
}

void SymbolTable::startSubroutine() {
    subroutineScope.clear();
    argumentCount = 0;
    localCount = 0;
}

void SymbolTable::define(const std::string& name, const std::string& type,
                         const std::string& kind) {
    const bool classKind = (kind == "static" || kind == "field");
    auto& scope = classKind ? classScope : subroutineScope;

    if (scope.find(name) != scope.end()) {
        throw std::runtime_error("Duplicate symbol declaration: '" + name + "'");
    }
    // Jack permits a subroutine variable to shadow a class variable, so only
    // check duplicates in the scope into which this symbol is being inserted.
    int index = 0;
    if (kind == "static") index = staticCount++;
    else if (kind == "field") index = fieldCount++;
    else if (kind == "argument") index = argumentCount++;
    else if (kind == "var" || kind == "local") index = localCount++;
    else throw std::runtime_error("Unknown symbol kind: '" + kind + "'");

    scope.emplace(name, Symbol{name, type, kind, index});
}

int SymbolTable::varCount(const std::string& kind) const {
    if (kind == "static") return staticCount;
    if (kind == "field") return fieldCount;
    if (kind == "argument") return argumentCount;
    if (kind == "var" || kind == "local") return localCount;
    return 0;
}

bool SymbolTable::lookup(const std::string& name, Symbol& result) const {
    auto sub = subroutineScope.find(name);
    if (sub != subroutineScope.end()) {
        result = sub->second;
        return true;
    }
    auto cls = classScope.find(name);
    if (cls != classScope.end()) {
        result = cls->second;
        return true;
    }
    return false;
}

bool SymbolTable::containsInCurrentSubroutine(const std::string& name) const {
    return subroutineScope.find(name) != subroutineScope.end();
}
