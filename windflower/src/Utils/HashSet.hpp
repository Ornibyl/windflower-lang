#ifndef WF_HASH_SET_HPP
#define WF_HASH_SET_HPP

#include <unordered_set>

#include "Allocate.hpp"

namespace wf
{
    template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
    using HashSet = std::unordered_set<Key, Hash, KeyEqual, StdAllocator<Key>>;
}

#endif