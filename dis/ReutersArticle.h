#pragma once

#include <azgra/string/smart_string_view.h>

namespace dis
{

    class ReutersArticle
    {
    private:
        std::vector<azgra::string::SmartStringView<char>> m_articleLines;
        std::vector<azgra::string::SmartStringView<char>> m_textLines;

    void parse_article();

    public:
        ReutersArticle() = default;

        explicit ReutersArticle(std::vector<azgra::string::SmartStringView<char>> &articleLines);
    };

}
