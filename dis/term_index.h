#pragma once

#include <azgra/string/smart_string_view.h>
#include <map>
#include <string>
#include <set>

namespace dis
{
    typedef size_t DocId;
    typedef std::map<std::string, std::set<DocId>> TermIndex;

    inline bool is_term(const azgra::string::SmartStringView<char> &str)
    {
        return ((str.length() > 2) && (!str.is_number()));
    }
}