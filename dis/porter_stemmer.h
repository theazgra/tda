#pragma once

#include <azgra/string/ascii_string.h>

using namespace azgra::string;

struct StemInfo
{
    AsciiString string;
    size_t endIndex;
    long offset = 0;

    StemInfo() = default;

    explicit StemInfo(const AsciiString& _string)
    {
        string = _string;
        endIndex = string.length() - 1;
    }
};

AsciiString stem_word(const char *string);

AsciiString stem_word(const char *string, const size_t len);

void test_porter_stemmer();
