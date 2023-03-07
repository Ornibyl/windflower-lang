#include "SymbolTable.hpp"

namespace wf
{
    SymbolTable::SymbolTable(State* state)
        : m_symbols(state)
    {
    }

    SymbolInfo& SymbolTable::create_variable(StringObject* name)
    {
        return m_symbols[name] = (SymbolInfo){
            .address = m_next_address++,
        };
    }

    const SymbolInfo& SymbolTable::get_symbol(StringObject* name) const
    {
        return m_symbols.at(name);
    }

}