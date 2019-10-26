#include "SgmlFile.h"

namespace dis
{
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
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Loaded %lu Reuter articles.\n", m_articles.size());
        always_assert(fromLine == -1 && "Missing closing of article");
    }

    ReutersArticle SgmlFile::create_article(const int fromLine, const int toLine)
    {
        std::vector<azgra::string::SmartStringView<char>> articleLines((toLine - fromLine) + 1);
        for (int i = 0; i <= (toLine-fromLine); ++i)
        {
            articleLines[i] = m_lineViews[fromLine + i];
        }
        return ReutersArticle(articleLines);
    }


}