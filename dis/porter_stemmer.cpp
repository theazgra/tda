#include "porter_stemmer.h"

using namespace azgra::string;

/// Check if the string ends with suffix. If true set the offset to the index before suffix.
/// \param si Stemmer info with the string.
/// \param string Suffix
/// \return The if the string ends with the given suffix.
static bool ends(StemInfo &si, const char *string)
{
    const size_t stringLen = strlen(string);
    if (stringLen > (si.endIndex + 1))
        return false;

    if (si.string.last_index_of(string) != static_cast<int>((si.endIndex - stringLen + 1)))
        return false;
    si.offset = si.endIndex - stringLen;
    return true;
}

/// Set suffix for porter string from (1 + offset) to string value
/// \param si Porter string.
/// \param string Suffix value.
static void set_end(StemInfo &si, const char *string)
{
    const size_t stringLen = strlen(string);
    for (size_t i = 0; i < stringLen; ++i)
    {
        si.string.at(1 + si.offset + i) = string[i];
    }
    si.endIndex = si.offset + stringLen;
}

/// Check if character at index i is consonant.
/// \param si Porter string.
/// \param i Index of character.
/// \return True if char at index is consonant or 'y' as the first char of the word or char after vowel
static bool is_consonant(const StemInfo &si, const size_t i)
{
    const char c = si.string.at(i);
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
    {
        return false;
    }
    else if (c == 'y')
    {
        if (i == 0)
        {
            return true;
        }
        else
        {
            assert(i >= 1);
            return !is_consonant(si, i - 1);
        }
    }
    return true;
}

/// Check if char at index i is vowel.
/// \param si Porter string.
/// \param i Character index.
/// \return True if char at index is vowel.
static bool is_vowel(const StemInfo &si, const size_t i)
{
    return !is_consonant(si, i);
}

/// Measure the consonant sequence count to the offset.
/// \param si Porter string.
/// \return m as (VC)^m in the porter string.
static size_t vc_seq_count(const StemInfo &si)
{
    if (si.offset < 0 || si.offset > static_cast<long>(si.endIndex))
        return 0;
    size_t result = 0;
    long i = 0;
    for (;;)
    {
        if (i > si.offset)
            return result;

        // Until we find vowel.
        if (!is_consonant(si, i))
            break;
        ++i;
    }
    ++i;
    for (;;)
    {
        for (;;)
        {
            if (i > si.offset)
                return result;

            // Until we find consonant.
            if (is_consonant(si, i))
                break;
            ++i;
        }
        ++i;
        ++result;
        for (;;)
        {
            if (i > si.offset)
                return result;

            // Until we find vowel.
            if (!is_consonant(si, i))
                break;
            ++i;
        }
        ++i;
    }

    return result;
}

/// Check if there is vowel in current porter string to the (offset + 1)
/// \param si Porter string.
/// \return True if string contains vowel.
static bool vowel_in_stem(const StemInfo &si)
{
    for (int i = 0; i < (si.offset + 1); ++i)
    {
        if (is_vowel(si, i))
            return true;
    }
    return false;
}

/// Check if there is double consonant at string[i-1],string[i]
/// \param si Porter string.
/// \param i Higher index.
/// \return True if there is double consonant.
static bool double_consonant(const StemInfo &si, const size_t i)
{
    if (i < 1)
        return false;
    if (si.string.at(i) != si.string.at(i - 1))
        return false;
    return is_consonant(si, i);
}

/// Check if string ends with CVC and the second C is not w,x,y
/// \param si Porter string.
/// \param i Index.
/// \return True if string ends with CVC.
static bool is_cvc_end(const StemInfo &si, const size_t i)
{
    if ((i < 2) || is_vowel(si, i - 2) || is_consonant(si, i - 1) || is_vowel(si, i))
        return false;

    const char c = si.string.at(i);
    return !(c == 'w' || c == 'x' || c == 'y');
}

/// Remove SS,IES,S,EED,ED,ING,AT,BL,IZ,Y
/// \param si Porter strng.
static void step_1abc(StemInfo &si)
{
    if (ends(si, "s"))
    {
        if (ends(si, "sses"))
        {
            si.endIndex -= 2;
        }
        else if (ends(si, "ies"))
        {
            set_end(si, "i");
        }
        else if (si.string.at(si.endIndex - 1) != 's')
        {
            si.endIndex -= 1;
        }
    }
    if (ends(si, "eed"))
    {
        if (vc_seq_count(si) > 0)
        {
            si.endIndex -= 1;
        }
    }
    else if ((ends(si, "ed") || ends(si, "ing")) && vowel_in_stem(si))
    {
        si.endIndex = si.offset;
        if (ends(si, "at"))
            set_end(si, "ate");
        else if (ends(si, "bl"))
            set_end(si, "ble");
        else if (ends(si, "iz"))
            set_end(si, "ize");
        else if (double_consonant(si, si.endIndex))
        {
            --si.endIndex;
            const char c = si.string.at(si.endIndex);
            if (c == 'l' || c == 's' || c == 'z')
                ++si.endIndex;
        }
        else if ((vc_seq_count(si) == 1) && is_cvc_end(si, si.endIndex))
            set_end(si, "e");
    }

    if (ends(si, "y") && vowel_in_stem(si))
    {
        set_end(si, "i");
    }
}

static void replace_end_if_m_gt0(StemInfo &si, const char *suffix)
{
    if (vc_seq_count(si) > 0)
        set_end(si, suffix);
}

static void remove_end_if_m_gt0(StemInfo &si)
{
    if (vc_seq_count(si) > 0)
        set_end(si, "");
}

static void step_2(StemInfo &si)
{
    if (ends(si, "ational"))
        replace_end_if_m_gt0(si, "ate");
    else if (ends(si, "tional"))
        replace_end_if_m_gt0(si, "tion");
    else if (ends(si, "enci"))
        replace_end_if_m_gt0(si, "ence");
    else if (ends(si, "anci"))
        replace_end_if_m_gt0(si, "ance");
    else if (ends(si, "izer"))
        replace_end_if_m_gt0(si, "ize");
    else if (ends(si, "abli"))
        replace_end_if_m_gt0(si, "able");
    else if (ends(si, "alli"))
        replace_end_if_m_gt0(si, "al");
    else if (ends(si, "entli"))
        replace_end_if_m_gt0(si, "ent");
    else if (ends(si, "eli"))
        replace_end_if_m_gt0(si, "e");
    else if (ends(si, "ousli"))
        replace_end_if_m_gt0(si, "ous");
    else if (ends(si, "ization"))
        replace_end_if_m_gt0(si, "ize");
    else if (ends(si, "ation"))
        replace_end_if_m_gt0(si, "ate");
    else if (ends(si, "ator"))
        replace_end_if_m_gt0(si, "ate");
    else if (ends(si, "alism"))
        replace_end_if_m_gt0(si, "al");
    else if (ends(si, "iveness"))
        replace_end_if_m_gt0(si, "ive");
    else if (ends(si, "fulness"))
        replace_end_if_m_gt0(si, "ful");
    else if (ends(si, "ousness"))
        replace_end_if_m_gt0(si, "ous");
    else if (ends(si, "aliti"))
        replace_end_if_m_gt0(si, "al");
    else if (ends(si, "iviti"))
        replace_end_if_m_gt0(si, "ive");
    else if (ends(si, "biliti"))
        replace_end_if_m_gt0(si, "ble");
}

static void step_3(StemInfo &si)
{
    if (si.string.at(si.endIndex) == 'e')
    {
        if (ends(si, "icate"))
            replace_end_if_m_gt0(si, "ic");
        else if (ends(si, "ative"))
            remove_end_if_m_gt0(si);
        else if (ends(si, "alize"))
            replace_end_if_m_gt0(si, "al");
    }
    else
    {
        if (ends(si, "iciti"))
            replace_end_if_m_gt0(si, "ic");
        else if (ends(si, "ical"))
            replace_end_if_m_gt0(si, "ic");
        else if (ends(si, "ful"))
            remove_end_if_m_gt0(si);
        else if (ends(si, "ness"))
            remove_end_if_m_gt0(si);
    }
}

static void step_4(StemInfo &si)
{
    if (si.string.at(si.endIndex - 1) == 'a')
    {
        if (ends(si, "al"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'c')
    {
        if (ends(si, "ence") || ends(si, "ance"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'e')
    {
        if (ends(si, "er"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'i')
    {
        if (ends(si, "ic"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'l')
    {
        if (ends(si, "able") || ends(si, "ible"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'n')
    {
        if (ends(si, "ant") || ends(si, "ement") || ends(si, "ment") || ends(si, "end"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'o')
    {
        if ((ends(si, "ion") && (
                (si.string.at(si.offset) == 's') || (si.string.at(si.offset) == 't'))
            ) ||
            ends(si, "ou"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 's')
    {
        if (ends(si, "ism"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 't')
    {
        if (ends(si, "ate") || ends(si, "iti"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'u')
    {
        if (ends(si, "ous"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'v')
    {
        if (ends(si, "ive"))
            void(0);
        else return;
    }
    else if (si.string.at(si.endIndex - 1) == 'z')
    {
        if (ends(si, "ize"))
            void(0);
        else return;
    }
    if (vc_seq_count(si) > 1)
    {
        si.endIndex = si.offset;
    }
}

static void step_5(StemInfo &si)
{
    si.offset = si.endIndex;
    if (si.string.at(si.endIndex) == 'e')
    {
        const size_t m = vc_seq_count(si);
        if ((m > 1) || ((m == 1) && (!is_cvc_end(si, si.endIndex - 1))))
        {
            --si.endIndex;
        }
    }
    if (si.string.at(si.endIndex) == 'l')
    {
        if (double_consonant(si, si.endIndex) && vc_seq_count(si) > 1)
        {
            --si.endIndex;
        }
    }
}


AsciiString stem_word(const char *string)
{
    AsciiString asciiString(string);
    StemInfo si(asciiString);

    // NOTE(Moravec): No change can be made to strings of size [0,2]
    if (si.string.length() < 3)
    {
        return asciiString;
    }

    step_1abc(si);
    step_2(si);
    step_3(si);
    step_4(si);
    step_5(si);

    return si.string.substring(0, si.endIndex + 1);
}


AsciiString stem_word(const char *string, const size_t len)
{
    AsciiString asciiString(string, len);
    StemInfo si(asciiString);

    // NOTE(Moravec): No change can be made to strings of size [0,2]
    if (si.string.length() < 3)
    {
        return asciiString;
    }

    step_1abc(si);
    step_2(si);
    step_3(si);
    step_4(si);
    step_5(si);

    return si.string.substring(0, si.endIndex + 1);
}

static void stem_test(const char *a, const char *b)
{
    auto stemResult = stem_word(a);
    if (!stemResult.equals(b))
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Red, "Failed to stem word %s[%s], got: %s\n",
                               a, b, stemResult.get_c_string());
    }
    else
    {
        azgra::print_colorized(azgra::ConsoleColor::ConsoleColor_Green, "Stem word %s[%s]\n", a, stemResult.get_c_string());
    }
}

void test_porter_stemmer()
{
    // 1a
    stem_test("caresses", "caress");
    stem_test("ponies", "poni");
    stem_test("ties", "ti");
    stem_test("caress", "caress");
    stem_test("cats", "cat");

    // 1b
    stem_test("feed", "feed");
    stem_test("agreed", "agree");
    stem_test("plastered", "plaster");
    stem_test("bled", "bled");
    stem_test("motoring", "motor");
    stem_test("sing", "sing");

    // 1b_1
    stem_test("conflated", "conflate");
    stem_test("troubling", "trouble");
    stem_test("sized", "size");
    stem_test("hopping", "hop");
    stem_test("tanned", "tan");
    stem_test("hissing", "hiss");
    stem_test("fizzed", "fizz");

    // 1c
    stem_test("happy", "happi");
    stem_test("sky", "sky");

    // 2
    stem_test("relational", "relate");
    stem_test("conditional", "condition");
    stem_test("rational", "rational");
    stem_test("valenci", "valence");
    stem_test("hesitanci", "hesit"); // hesitance
    stem_test("digitizer", "digit"); // digitize
    stem_test("radicalli", "radic"); // radical
    stem_test("vileli", "vile");
    stem_test("analogousli", "analog"); // analogous
    stem_test("operator", "oper"); // operate
    stem_test("decisiveness", "decis"); // decisive
    stem_test("hopefulness", "hope");
    stem_test("callousness", "callous");
    stem_test("formaliti", "formal");
    stem_test("sensitiviti", "sensit"); // sensitive
    stem_test("vietnamization", "vietnam"); //vietnamize

    // 3
    stem_test("triplicate", "triplic");
    stem_test("formative", "form");
    stem_test("formalize", "formal");
    stem_test("electriciti", "electr"); //electric
    stem_test("electrical", "electr");
    stem_test("hopeful", "hope");
    stem_test("goodness", "good");

    // 4
    stem_test("revival", "reviv");
    stem_test("allowance", "allow");
    stem_test("inference", "infer");
    stem_test("airliner", "airlin");
    stem_test("gyroscopic", "gyroscop");
    stem_test("adjustable", "adjust");
    stem_test("defensible", "defens");
    stem_test("irritant", "irrit");
    stem_test("replacement", "replac");
    stem_test("adjustment", "adjust");
    stem_test("dependant", "depend");
    stem_test("adoption", "adopt");
    stem_test("adopsion", "adops");
    stem_test("homologou", "homolog");
    stem_test("communism", "commun");
    stem_test("activate", "activ");
    stem_test("angulariti", "angular");
    stem_test("homologous", "homolog");
    stem_test("effective", "effect");
    stem_test("bowdlerize", "bowdler");

    // 5
    stem_test("probate", "probat");
    stem_test("rate", "rate");
    stem_test("cease", "ceas");
    stem_test("controll", "control");
    stem_test("roll", "roll");
    stem_test("warcraft", "");
}


