#pragma once

#include "term_index.h"
#include <azgra/collection/enumerable.h>
#include <azgra/string/smart_string_view.h>
#include <sstream>
#include <azgra/collection/vector_linq.h>
#include "porter_stemmer.h"

namespace dis
{
    class ReutersArticle
    {
    private:
        typedef azgra::string::SmartStringView<char> AsciiTextView;
        static constexpr char LowerCaseOffset = 'a' - 'A';
        std::vector<AsciiTextView> m_articleLines;
        std::vector<AsciiTextView> m_articleTextLines;

        void filter_line(std::stringstream &ss, const AsciiTextView &line, const std::vector<AsciiTextView> &stopwords) const;

        void parse_article();
        std::string m_processedText;
        std::vector<AsciiTextView> m_processedWords;
        DocId m_docId;

    public:
        explicit ReutersArticle(const DocId id);
        explicit ReutersArticle(const DocId id, std::vector<AsciiTextView> &articleLines);
        void filter_article_text(const std::vector<azgra::string::SmartStringView<char>> &stopwords);

        void extract_filtered_article_text(std::stringstream &textStream, const std::vector<AsciiTextView> &stopwords) const;

        [[nodiscard]] std::string const& get_processed_string() const;

        void destroy_views();

        void index_article_terms(TermIndex &index) const;

        DocId get_docId() const;
    };

}
