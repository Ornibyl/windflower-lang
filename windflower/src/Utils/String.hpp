#ifndef WF_STRING_HPP
#define WF_STRING_HPP

#include <string>

#include "Allocate.hpp"

namespace wf
{
    using String = std::basic_string<char, std::char_traits<char>, StdAllocator<char>>;
}

#endif