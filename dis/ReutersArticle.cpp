#include "ReutersArticle.h"

namespace dis
{
    ReutersArticle::ReutersArticle(std::vector<AsciiTextView> &articleLines)
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
                m_articleTextLines.push_back(m_articleLines[line]);
            }
        }
    }

    std::string ReutersArticle::extract_filtered_article_text(const std::vector<AsciiTextView> &stopwords) const
    {
        std::stringstream textStream;

        for (const auto &line : m_articleTextLines)
        {
            filter_line(textStream, line, stopwords);
            textStream << '\n';
        }

        return textStream.str();
    }

    void ReutersArticle::filter_line(std::stringstream &ss, const ReutersArticle::AsciiTextView &line,
                                     const std::vector<AsciiTextView> &stopwords) const
    {
        std::stringstream lineStream;
        for (const auto &c : line)
        {
            if ((c >= 'a' && c <= 'z') || (c == ' ') || (c >= '0' && c <= '9'))
                lineStream << c;
            else if (c >= 'A' && c <= 'Z')
                lineStream << static_cast<char>(c + LowerCaseOffset);
            else if (c == '<' || c == '>') // Force word separation at angle brackets.
                lineStream << ' ';
        }
        const auto lineString = lineStream.str();
        const auto words = azgra::string::SmartStringView<char>(lineString.c_str()).split(' ');
        for (const auto word : words)
        {
            if (word.is_empty())
                continue;
            if (!azgra::collection::contains(stopwords.begin(), stopwords.end(), word))
            {
                ss << word.string_view() << ' ';
            }
        }
    }
}
