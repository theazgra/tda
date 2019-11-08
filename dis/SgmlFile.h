#pragma once

#include <azgra/io/text_file_functions.h>
#include "ReutersArticle.h"

namespace dis
{
    inline std::vector<azgra::string::SmartStringView<char>> strings_to_views(const std::vector<std::string> &strings)
    {
        std::vector<azgra::string::SmartStringView<char>> views(strings.size());
        for (size_t i = 0; i < strings.size(); ++i)
        {
            views[i] = azgra::string::SmartStringView<char>(strings[i].c_str());
        }
        return views;
    }

    class SgmlFile
    {
    private:
        std::vector<std::string> m_lines;
        std::vector<azgra::string::SmartStringView<char>> m_lineViews;
        std::vector<ReutersArticle> m_articles;


        ReutersArticle create_article(const int fromLine, const int toLine, const DocId id);

        const char *m_fileName{};

        void parse(DocId &docId);

        void load_articles(DocId &docId);

    public:
        SgmlFile() = default;

        static SgmlFile load(const char *fileName, DocId &docId);

        void preprocess_article_text(const std::vector<azgra::string::SmartStringView<char>> &stopwords);

        void save_preprocessed_text(const char *fileName, const char *stopwordFile);

        std::vector<ReutersArticle> get_articles();

        void destroy_original_text();

        void index_atricles(TermIndex &index) const;
    };
}

