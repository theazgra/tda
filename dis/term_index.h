#pragma once

#include <map>
#include <string>
#include <set>

namespace dis
{
    typedef size_t DocId;
    typedef std::map<std::string, std::set<DocId>> TermIndex;
}