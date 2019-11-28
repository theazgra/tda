#pragma once

#include <azgra/io/binary_file_functions.h>
#include <azgra/io/stream/in_binary_file_stream.h>
#include <azgra/collection/set_utilities.h>
#include <azgra/io/stream/memory_bit_stream.h>
#include "SgmlFile.h"
#include "term_index.h"
#include "vector_model.h"

namespace dis
{
    void test();

    std::vector<size_t> generate_fibonacci_sequence(const size_t n);


    struct SizedIndexEntry
    {
        std::set<DocId> documents;
        size_t size;

        SizedIndexEntry() = default;

        explicit SizedIndexEntry(const std::set<DocumentOccurence> &occurencies)
        {
            auto docIds = azgra::collection::select(occurencies.begin(),
                                                    occurencies.end(),
                                                    [](const DocumentOccurence &occurence)
                                                    { return occurence.docId; });

            documents = std::set<DocId>(docIds.begin(), docIds.end());
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
        size_t documentCount=0;

        VectorModel m_vectorModel;

    public:
        explicit SgmlFileCollection(std::vector<const char *> sgmlFilePaths);

        void load_and_preprocess_sgml_files(const char *stopwordFile);

        void create_term_index_with_vector_model();

        void dump_index(const char *path);

        void load_index(const char *path);

        void save_preprocessed_documents(const char *path);

        QueryResult query(azgra::string::SmartStringView<char> &queryText, const bool verbose) const;

        void dump_compressed_index(const char *filePath) const;

        void load_compressed_index(const char *filePath);

        VectorModel const &get_vector_model() const;
    };
}
