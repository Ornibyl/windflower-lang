#ifndef WF_BYTECODE_HPP
#define WF_BYTECODE_HPP

#include "Object.hpp"
#include "Utils/String.hpp"

namespace wf
{
    String disassemble_bytecode_object(State* const state, BytecodeObject* code);
}

#endif