#ifndef WF_STATE_HPP
#define WF_STATE_HPP

#include "Windflower/Windflower.hpp"
#include "Vm/VmStack.hpp"
#include "Vm/Vm.hpp"
#include "Utils/HashSet.hpp"
#include <stack>

namespace wf
{
    struct InternTableEquality
    {
        using is_transparent = void;

        bool operator()(wf::StringObject* left, wf::StringObject* right) const
        {
            return left == right;
        }

        bool operator()(wf::StringObject* left, const wf::StringInfo& right) const
        {
            return left->length == right.length
                && left->hash == right.hash
                && std::memcmp(left->text, right.text, left->length) == 0;
        }

        bool operator()(const wf::StringInfo& left, wf::StringObject* right) const
        {
            return left.length == right->length
                && left.hash == right->hash
                && std::memcmp(left.text, right->text, left.length) == 0;
        }
    };

    struct State
    {
        State(const EnvironmentCreateInfo& create_info);
        ~State();
        Allocator& allocator;
        Object* allocated_objects = nullptr;

        HashSet<StringObject*, std::hash<StringObject*>, InternTableEquality> interned_strings;

        Vm vm;
        VmStack stack;
    };
}

#endif