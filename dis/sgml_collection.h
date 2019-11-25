#pragma once

#include <azgra/io/binary_file_functions.h>
#include <azgra/stream/in_binary_file_stream.h>
#include <azgra/collection/set_utilities.h>
#include <azgra/stream/memory_bit_stream.h>
#include "SgmlFile.h"
#include "term_index.h"

namespace dis
{
    void test();

    std::vector<size_t> generate_fibonacci_sequence(const size_t n);

    struct QueryResult
    {
        std::set<DocId> documents;
    };

    enum QueryTermType
    {
        AND,
        OR,
        NOT
    };

    struct SizedIndexEntry
    {
        std::set<DocId> documents;
        size_t size;
        QueryTermType type;

        SizedIndexEntry() = default;

        explicit SizedIndexEntry(const std::set<DocId> &_documents, QueryTermType termType)
        {
            documents = _documents;
            size = documents.size();
            type = termType;
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

        QueryResult query(azgra::string::SmartStringView<char> &queryText, const bool verbose) const;

        void dump_compressed_index(const char *filePath) const;

        void load_compressed_index(const char *filePath);
    };
}
