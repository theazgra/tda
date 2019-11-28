#pragma once

#include <azgra/string/smart_string_view.h>
#include <map>
#include <string>
#include <set>

namespace dis
{
    typedef size_t DocId;

    struct DocumentOccurence
    {
        DocId docId = 0;
        size_t occurenceCount = 0;

        DocumentOccurence() = default;

        explicit DocumentOccurence(const DocId id, const size_t count) : docId(id), occurenceCount(count)
        {
            occurenceCount = 1;
        }

        bool operator<(const DocumentOccurence &other) const
        {
            return (docId < other.docId);
        }

        bool operator==(const DocumentOccurence &other) const
        {
            return (docId == other.docId);
        }

    };


    typedef std::map<std::string, std::set<DocumentOccurence>> TermIndex;

    inline bool is_term(const azgra::string::SmartStringView<char> &str)
    {
        return ((str.length() > 2) && (!str.is_number()));
    }

    struct QueryResult
    {
        std::set<DocId> documents;
    };
}