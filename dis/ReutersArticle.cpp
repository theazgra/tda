#include "ReutersArticle.h"

namespace dis
{

    ReutersArticle::ReutersArticle(const DocId id)
    {
        m_docId = id;
    }

    ReutersArticle::ReutersArticle(const DocId id, std::vector<AsciiTextView> &articleLines)
    {
        m_docId = id;
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

    void ReutersArticle::filter_article_text(const std::vector<azgra::string::SmartStringView<char>> &stopwords)
    {
        std::stringstream articleStream;
        for (const auto &line : m_articleTextLines)
        {
            filter_line(articleStream, line, stopwords);
            articleStream << '\n';
        }
        m_processedText = articleStream.str();
        m_processedWords = azgra::string::SmartStringView<char>(m_processedText.c_str()).multi_split({' ', '\n'});
    }

    void ReutersArticle::extract_filtered_article_text(std::stringstream &textStream,
                                                       const std::vector<AsciiTextView> &stopwords) const
    {
        textStream << "\n-------- ARTICLE --------\n";

        for (const auto &line : m_articleTextLines)
        {
            filter_line(textStream, line, stopwords);
            textStream << '\n';
        }

        textStream << "------- END OF ARTICLE -----\n";
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
                AsciiString str = stem_word(word.data(), word.length());
                ss << str.get_c_string() << ' ';
            }
        }
    }

    std::string const &ReutersArticle::get_processed_string() const
    {
        return m_processedText;
    }

    void ReutersArticle::destroy_views()
    {
        m_articleLines.clear();
        m_articleTextLines.clear();
    }

    void ReutersArticle::index_article_terms(TermIndex &index) const
    {
        std::string wordKey;
        for (const auto &word : m_processedWords)
        {
            if (word.is_empty() || word.equals(" "))
                continue;

            wordKey = std::string(word.string_view());
            if (index.find(wordKey) == index.end())
            {
                index[wordKey] = {m_docId};
            }
            else
            {
                index[wordKey].insert(m_docId);
            }
        }
    }


}
