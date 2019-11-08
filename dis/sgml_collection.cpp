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

        DocId docId = 0;
        m_sgmlFiles.resize(m_inputFilePaths.size());
        for (size_t fileIndex = 0; fileIndex < m_inputFilePaths.size(); ++fileIndex)
        {
            m_sgmlFiles[fileIndex] = SgmlFile::load(m_inputFilePaths[fileIndex], docId);
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

    void SgmlFileCollection::load_index(const char *path)
    {
        std::vector<std::pair<std::string, std::set<DocId>>> mapPairs;
        std::function<std::pair<std::string, std::set<DocId>>(const azgra::string::SmartStringView<char> &)> fn =
                [](const azgra::string::SmartStringView<char> &line)
                {
                    auto index = line.index_of(":");
                    std::string term = std::string(line.substring(0, index).string_view());
                    auto docIds = line.substring(index + 1).split(',');
                    std::set<DocId> ids;
                    for (const auto dIdStr : docIds)
                    {
                        if (!dIdStr.is_empty())
                        {
                            ids.insert(atol(dIdStr.data()));
                        }
                    }
                    return std::make_pair(term, ids);
                };

        mapPairs = azgra::io::parse_by_lines<std::pair<std::string, std::set<DocId>>>(path, fn);
        m_index = TermIndex(mapPairs.begin(), mapPairs.end());
        fprintf(stdout, "%lu\n", mapPairs.size());
    }

    void SgmlFileCollection::save_preprocessed_documents(const char *path)
    {
        std::ofstream fStream(path, std::ios::out);
        always_assert(fStream.is_open());

        for (auto &sgmlFile : m_sgmlFiles)
        {
            for (auto &article : sgmlFile.get_articles())
            {
                fStream << "DocId:" << article.get_docId() << '\n';
                fStream << article.get_processed_string() << '\n';
            }
        }
    }

    QueryResult SgmlFileCollection::query(azgra::string::SmartStringView<char> &queryText) const
    {
        QueryResult result = {};
        if (queryText.is_empty())
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Query string is empty.\n");
            return result;
        }
        if (m_index.empty())
        {
            azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Index wasn't created nor loaded.\n");
            return result;
        }

        auto keywords = queryText.split(" ");
        std::vector<SizedIndexEntry> indexEntries;
        for (const auto &keyword : keywords)
        {

            AsciiString str = stem_word(keyword.data(), keyword.length());
            const std::string key = std::string(str.get_c_string());
            if (keyword.is_empty() || (m_index.find(key) == m_index.end()))
                continue;

            indexEntries.push_back(SizedIndexEntry(m_index.at(key))); // NOLINT(hicpp-use-emplace,modernize-use-emplace)
        }

        std::sort(indexEntries.begin(), indexEntries.end());

        //result.documents = indexEntries[0].documents;
        std::vector<DocId> unionVector = std::vector<DocId>(indexEntries[0].documents.begin(),
                                                            indexEntries[0].documents.end());
        if (indexEntries.size() > 1)
        {
            for (size_t i = 1; i < indexEntries.size(); ++i)
            {
                std::vector<DocId> unionResult;
                unionResult.clear();
                std::set_intersection(unionVector.begin(), unionVector.end(),
                                      indexEntries[i].documents.begin(),
                                      indexEntries[i].documents.end(),
                                      std::back_inserter(unionResult));
                unionVector = unionResult;
            }
        }
        result.documents = std::set<DocId>(unionVector.begin(), unionVector.end());
        return result;
    }

}