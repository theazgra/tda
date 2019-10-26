#include "ReutersArticle.h"

namespace dis
{
    ReutersArticle::ReutersArticle(std::vector<azgra::string::SmartStringView<char>> &articleLines)
    {
        m_articleLines = std::move(articleLines);
        parse_article();
    }

    void ReutersArticle::parse_article()
    {
        bool insideText = false;
        for (size_t line = 0; line < m_articleLines.size(); ++line)
        {
            if (m_articleLines[line].starts_with("<TEXT"))
            {
                insideText = true;
                continue;
            }
            if (m_articleLines[line].contains("</TEXT>"))
            {
                insideText = false;
                continue;
            }
            if (insideText)
            {
                m_textLines.push_back(m_articleLines[line]);
            }
        }
    }
}
