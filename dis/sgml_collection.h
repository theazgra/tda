#pragma once

#include "SgmlFile.h"
#include "term_index.h"

namespace dis
{
    class SgmlFileCollection
    {
    private:
        std::vector<const char *> m_inputFilePaths;
        std::vector<SgmlFile> m_sgmlFiles;
        TermIndex m_index;
    public:
        explicit SgmlFileCollection(std::vector<const char*> sgmlFilePaths);
        void load_and_preprocess_sgml_files(const char *stopwordFile);
        void extract_documents(const char *mergedDocumentFile);
        void create_term_index();

        void dump_index(const char* path);
        void load_index(const char* path);

        void save_preprocessed_documents(const char* path);

    };
}
