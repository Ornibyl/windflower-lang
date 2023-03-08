#ifndef WF_SYMBOL_TABLE_HPP
#define WF_SYMBOL_TABLE_HPP

#include <cstdint>

#include "Vm/Object.hpp"
#include "Utils/HashMap.hpp"

namespace wf
{
    using RegisterAddress = std::uint32_t;

    struct SymbolInfo
    {
        TypeId storage_type = TypeId::VOID;
        RegisterAddress address;
    };

    class SymbolTable
    {
    public:
        SymbolTable(State* state);

        std::uint32_t get_stack_symbol_count() const { return static_cast<std::uint32_t>(m_symbols.size()); }
        SymbolInfo& create_variable(StringObject* name);
        const SymbolInfo& get_symbol(StringObject* name) const;

        decltype(auto) find(StringObject* name) { return m_symbols.find(name); }
        decltype(auto) find(StringObject* name) const { return m_symbols.find(name); }

        decltype(auto) begin() { return m_symbols.begin(); }
        decltype(auto) begin() const { return m_symbols.begin(); }
        decltype(auto) cbegin() const { return m_symbols.cbegin(); }

        decltype(auto) end() { return m_symbols.end(); }
        decltype(auto) end() const { return m_symbols.end(); }
        decltype(auto) cend() const { return m_symbols.cend(); }
    private:
        RegisterAddress m_next_address = 0;
        HashMap<StringObject*, SymbolInfo> m_symbols;
    };
}

#endif