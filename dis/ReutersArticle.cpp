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
            else if (c == '<' || c == '>' || c == '/' || c == '\\' || c == '.' || c == '-') // Force word separation at some characters
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
        std::map<std::string, size_t> articleTerms;
        std::string key;
        for (const auto &word : m_processedWords)
        {
            if (!is_term(word))
                continue;

            key = std::string(word.string_view());
            if (articleTerms.find(key) == articleTerms.end())
            {
                articleTerms[key] = 1;
            }
            else
            {
                articleTerms[key] += 1;
            }
        }

        for (const auto&[key, occurenceCount] : articleTerms)
        {
            if (index.find(key) == index.end())
            {
                index[key] = {DocumentOccurence(m_docId, occurenceCount)};
            }
            else
            {
                index[key].insert(DocumentOccurence(m_docId, occurenceCount));
            }
        }
#if 0
        std::string wordKey;
        for (const auto &word : m_processedWords)
        {
            if (!is_term(word))
                continue;

            wordKey = std::string(word.string_view());
            if (index.find(wordKey) == index.end())
            {
                // Creating new key in map.
                index[wordKey] = {m_docOccurence};
                //index[wordKey] = {m_docId};
            }
            else
            {
                // Key is already in map.

                auto ocurrenceIt = index[wordKey].find(m_docOccurence);
                // Wasnt yet found in this document.
                if (ocurrenceIt == index[wordKey].end())
                {
                    // Key doesnt have entry of this document.
                    index[wordKey].insert(m_docOccurence);
                }
                else
                {
                    // Key does have entry of this document.
                    //(*index[wordKey].find(m_docOccurence)).occurenceCount++;
                    //index[wordKey].insert(m_docId);
                }


            }
        }
#endif
    }

    DocId ReutersArticle::get_docId() const
    {
        return m_docId;
    }


}
