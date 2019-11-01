#pragma once

#include <azgra/string/ascii_string.h>

using namespace azgra::string;

struct StemInfo
{
    AsciiString *string;
    size_t  endIndex;
    size_t offset = 0;

    StemInfo() = default;

    explicit StemInfo(AsciiString *_string)
    {
        string = _string;
        endIndex = string->length() - 1;
    }
};

AsciiString stem_word(const char *string);

void test_porter_stemmer();
