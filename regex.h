#pragma once

#include <vector>
#include <azgra/string/smart_string_view.h>

namespace regex
{
    const char OrSymbol = '|';
    const char IterSymbol = '*';
    struct RegEx
    {
        virtual ~RegEx(){}
    };

    struct PairRegEx : RegEx
    {
        RegEx *a;
        RegEx *b;
    };

    struct RegExSymbol : RegEx
    {
        char symbol{};
        RegExSymbol() = default;
        explicit RegExSymbol(char s) : symbol(s) {}
    };

    struct RegExIter : RegEx
    {
        RegEx *innerEx;
    };

    struct RegExConcat : PairRegEx
    {
    };

    struct RegExOr : PairRegEx
    {
    };

    struct RegularExpression
    {
        std::vector<RegEx *> tokens;
        RegEx *expression;

        void simple_parse(azgra::string::SmartStringView<char> exp);
    };
}
