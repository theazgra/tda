#pragma once

#include <azgra/io/text_file_functions.h>
#include "ReutersArticle.h"

namespace dis
{
    class SgmlFile
    {
    private:
        SgmlFile() = default;

        std::vector<std::string> m_lines;
        std::vector<azgra::string::SmartStringView<char>> m_lineViews;
        std::vector<ReutersArticle> m_articles;


        ReutersArticle create_article(const int fromLine, const int toLine);
        const char* m_fileName{};
        void parse();
        void load_articles();

    public:
        static SgmlFile load(const char *fileName);

        void save_preprocessed_text(const char *fileName, const char *stopwordFile);
    };
}

