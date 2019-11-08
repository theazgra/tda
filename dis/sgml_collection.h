#pragma once

#include "SgmlFile.h"
#include "term_index.h"

namespace dis
{
    struct QueryResult
    {
        std::set<DocId> documents;
    };

    struct SizedIndexEntry
    {
        std::set<DocId> documents;
        size_t size;

        SizedIndexEntry() = default;

        explicit SizedIndexEntry(const std::set<DocId> &_documents)
        {
            documents = _documents;
            size = documents.size();
        }

        bool operator<(const SizedIndexEntry &other) const
        {
            return (size < other.size);
        }
    };

    class SgmlFileCollection
    {
    private:
        std::vector<const char *> m_inputFilePaths;
        std::vector<SgmlFile> m_sgmlFiles;
        TermIndex m_index;
    public:
        explicit SgmlFileCollection(std::vector<const char *> sgmlFilePaths);

        void load_and_preprocess_sgml_files(const char *stopwordFile);

        void extract_documents(const char *mergedDocumentFile);

        void create_term_index();

        void dump_index(const char *path);

        void load_index(const char *path);

        void save_preprocessed_documents(const char *path);

        QueryResult query(azgra::string::SmartStringView<char> &queryText) const;

    };
}
