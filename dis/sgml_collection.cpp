#include "sgml_collection.h"

namespace dis
{
    SgmlFileCollection::SgmlFileCollection(std::vector<const char *> sgmlFilePaths)
    {
        m_inputFilePaths = std::move(sgmlFilePaths);
    }

    void SgmlFileCollection::load_and_preprocess_sgml_files(const char *stopwordFile)
    {
        const auto stopwords = strings_to_views(azgra::io::read_lines(stopwordFile));

        m_sgmlFiles.resize(m_inputFilePaths.size());
        for (size_t fileIndex = 0; fileIndex < m_inputFilePaths.size(); ++fileIndex)
        {
            m_sgmlFiles[fileIndex] = SgmlFile::load(m_inputFilePaths[fileIndex]);
            m_sgmlFiles[fileIndex].preprocess_article_text(stopwords);
            m_sgmlFiles[fileIndex].destroy_original_text();
        }
    }

    void SgmlFileCollection::extract_documents(const char *mergedDocumentFile)
    {
        // sgmlFile.save_preprocessed_text(outputFile, stopwordsFile);
    }

    void SgmlFileCollection::create_term_index()
    {
        m_index.clear();
        for (const auto &sgmlFile : m_sgmlFiles)
        {
            sgmlFile.index_atricles(m_index);
        }
        fprintf(stdout, "Created index with %lu terms\n", m_index.size());
        auto pair = *m_index.begin();
        fprintf(stdout, "Term %s in %lu documents\n", pair.first.c_str(), pair.second.size());

    }

    void SgmlFileCollection::dump_index(const char *path)
    {
        std::ofstream dump(path, std::ios::out);
        for (const auto &term : m_index)
        {
            dump << term.first << ':';
            for (const DocId docId : term.second)
            {
                dump << docId << ",";
            }
            dump << "\n";
        }
    }

}