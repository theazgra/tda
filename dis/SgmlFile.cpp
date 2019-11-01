#include <azgra/collection/enumerable.h>
#include "SgmlFile.h"

namespace dis
{
    static std::vector<azgra::string::SmartStringView<char>> strings_to_views(const std::vector<std::string> &strings)
    {
        std::vector<azgra::string::SmartStringView<char>> views(strings.size());
        for (size_t i = 0; i < strings.size(); ++i)
        {
            views[i] = azgra::string::SmartStringView<char>(strings[i].c_str());
        }
        return views;
    }

    SgmlFile SgmlFile::load(const char *fileName)
    {
        SgmlFile file;
        file.m_fileName = fileName;
        file.parse();
        return file;
    }

    void SgmlFile::parse()
    {
        m_lines = azgra::io::read_lines(m_fileName);
        m_lineViews.resize(m_lines.size());
        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            m_lineViews[i] = azgra::string::SmartStringView<char>(m_lines[i]);
        }

        always_assert(m_lines[0] == "<!DOCTYPE lewis SYSTEM \"lewis.dtd\">" && "Wrong Sgml file header.");
        load_articles();
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Loaded %lu Reuter articles from %s.\n", m_articles.size(),
                               m_fileName);
    }

    void SgmlFile::load_articles()
    {
        int fromLine = -1;
        for (size_t line = 0; line < m_lines.size(); ++line)
        {
            if (m_lineViews[line].starts_with("<REUTERS "))
            {
                fromLine = line;
                continue;
            }

            if (m_lineViews[line].starts_with("</REUTERS>"))
            {
                always_assert(fromLine != -1);
                m_articles.push_back(create_article(fromLine, line));
                fromLine = -1;
            }
        }

        always_assert(fromLine == -1 && "Missing closing of article");
    }

    ReutersArticle SgmlFile::create_article(const int fromLine, const int toLine)
    {
        std::vector<azgra::string::SmartStringView<char>> articleLines((toLine - fromLine) + 1);
        for (int i = 0; i <= (toLine - fromLine); ++i)
        {
            articleLines[i] = m_lineViews[fromLine + i];
        }
        return ReutersArticle(articleLines);
    }

    void SgmlFile::save_preprocessed_text(const char *fileName, const char *stopwordFile)
    {
        const auto stopwords = azgra::io::read_lines(stopwordFile);
        const auto stopwordViews = strings_to_views(stopwords);
        std::ofstream ofStream(fileName, std::ios::out);
        always_assert(ofStream.is_open());

        std::stringstream processedTextStream;
        for (const auto &article : m_articles)
        {
            article.extract_filtered_article_text(processedTextStream, stopwordViews);
        }
        ofStream << processedTextStream.str();
        //ofStream.write(filteredText.c_str(), filteredText.length());
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green,
                               "Saved preprocessed text to %s.\nUsed stopwords from: %s\n",
                               fileName,
                               stopwordFile);
    }


}