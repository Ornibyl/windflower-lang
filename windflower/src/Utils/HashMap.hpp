#ifndef WF_HASH_MAP_HPP
#define WF_HASH_MAP_HPP

#include <unordered_map>

#include "Allocate.hpp"

namespace wf
{
    template<typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
    using HashMap = std::unordered_map<Key, Value, Hash, KeyEqual, StdAllocator<std::pair<const Key, Value>>>;
}

#endif