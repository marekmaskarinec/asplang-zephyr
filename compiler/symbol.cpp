//
// Asp symbol table implementation.
//

#include "symbol.hpp"
#include "symbols.h"
#include <utility>

using namespace std;

SymbolTable::SymbolTable(bool reserveSystemSymbols)
{
    // Reserve symbols used by the system if applicable.
    if (reserveSystemSymbols)
    {
        Symbol(AspSystemModuleName);
        Symbol(AspSystemArgumentsName);
        Symbol(AspSystemMainModuleName);
    }
}

int32_t SymbolTable::Symbol(const string &name)
{
    // For a temporary, return a new negative symbol.
    if (name.empty())
    {
        if (nextUnnamedSymbol == 0)
            throw string("Maximum number of temporary symbols exceeded");
        auto result = nextUnnamedSymbol;
        if (nextUnnamedSymbol == INT32_MIN)
            nextUnnamedSymbol = 0;
        else
            nextUnnamedSymbol--;
        return result;
    }

    // Return a unique symbol for the given name.
    if (nextNamedSymbol == 0 && !symbolsByName.empty())
        throw string("Maximum number of name symbols exceeded");
    auto result = symbolsByName.insert(make_pair(name, nextNamedSymbol));
    if (result.second)
    {
        if (nextNamedSymbol == INT32_MAX)
            nextNamedSymbol = 0;
        else
            nextNamedSymbol++;
    }
    return result.first->second;
}

bool SymbolTable::IsDefined(const string &name) const
{
    return symbolsByName.find(name) != symbolsByName.end();
}

SymbolTable::Map::const_iterator SymbolTable::Begin() const
{
    return symbolsByName.begin();
}

SymbolTable::Map::const_iterator SymbolTable::End() const
{
    return symbolsByName.end();
}
