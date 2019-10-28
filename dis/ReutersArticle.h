#pragma once

#include <azgra/string/smart_string_view.h>
#include <sstream>
#include <azgra/collection/vector_linq.h>

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

    public:
        ReutersArticle() = default;

        explicit ReutersArticle(std::vector<AsciiTextView> &articleLines);

        [[nodiscard]] std::string extract_filtered_article_text(const std::vector<AsciiTextView> &stopwords) const;
    };

}
