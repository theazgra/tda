#include <fstream>
#include <string_view>
#include <cassert>
#include <cstdio>
#include <vector>
#include <cstring>
#include "Stopwatch.h"
#include "automata.h"

#define CHAR_COUNT 256
using namespace std;

std::string read_file(const char *file)
{
    std::ifstream fs(file, std::ios::in | std::ios::binary | std::ios::ate);
    assert(fs.is_open());

    auto fSize = fs.tellg();
    fs.seekg(std::ios::beg);

    std::vector<char> fData(fSize);
    fs.read(fData.data(), fSize);
    return std::string(fData.data(), fSize);
}

std::vector<size_t> brute_force(const string_view &text, const string_view &pattern)
{
    std::vector<size_t> result;

    size_t textLen = text.length();
    size_t patternLen = pattern.length();

    if (patternLen > textLen)
        return result;


    for (size_t textIndex = 0; textIndex < textLen; ++textIndex)
    {
        if ((textLen - textIndex) < patternLen)
            break;

        bool found = true;
        for (size_t patternIndex = 0; patternIndex < patternLen; ++patternIndex)
        {
            if (text[textIndex + patternIndex] != pattern[patternIndex])
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            result.push_back(textIndex);
        }
    }
    return result;
}

std::vector<long> preprocess_bc(const string_view &pattern)
{
    std::vector<long> bc(CHAR_COUNT);
    size_t patternLen = pattern.size();
    for (size_t i = 0; i < CHAR_COUNT; ++i)
    {
        bc[i] = patternLen;
    }
    for (size_t i = 0; i < patternLen - 1; ++i)
    {
        bc[pattern[i]] = (long) patternLen - i - 1;
    }
    return bc;
}

std::vector<size_t> horspool(const string_view &text, const string_view &pattern)
{
    std::vector<size_t> result;
    const size_t textLen = text.length();
    const size_t patternLen = pattern.length();
    if (patternLen > textLen)
        return result;
    std::vector<long> bc = preprocess_bc(pattern);
    long j = 0;
    char c;
    while (j <= (textLen - patternLen))
    {
        c = text[j + patternLen - 1];
        if (pattern[patternLen - 1] == c &&
            (std::memcmp(pattern.data(), text.data() + j * sizeof(char), patternLen - 1) == 0))
        {
            result.push_back(j);
        }
        j += bc[c];
    }
    return result;
}

std::vector<long> preprocess_kmp(const string_view &pattern)
{
    std::vector<long> kmpNext(pattern.length() + 1);

    const size_t patternLen = pattern.length();
    long i = 0;
    long j = -1;
    kmpNext[0] = -1;
    while (i < patternLen)
    {
        while (j > -1 && pattern[i] != pattern[j])
        {
            j = kmpNext[j];
        }
        ++i;
        ++j;
        if (pattern[i] == pattern[j])
        {
            kmpNext[i] = kmpNext[j];
        }
        else
        {
            kmpNext[i] = j;
        }
    }
    return kmpNext;
}

std::vector<size_t> knuth_morris_pratt(const string_view &text, const string_view &pattern)
{
    std::vector<size_t> result;
    const size_t textLen = text.length();
    const size_t patternLen = pattern.length();

    if (patternLen > textLen)
        return result;

    std::vector<long> kmpNext = preprocess_kmp(pattern);
    long i = 0;
    long j = 0;

    while (j < textLen)
    {
        while (i > -1 && pattern[i] != text[j])
        {
            i = kmpNext[i];
        }
        ++i;
        ++j;
        if (i >= patternLen)
        {
            result.push_back(j - i);
            i = kmpNext[i];
        }
    }
    return result;
}

int main(int argc, char **argv)
{
//    if (argc < 3)
//    {
//        fprintf(stdout, "[text file] [pattern]\n");
//        return 1;
//    }
//    auto text = read_file(argv[1]);
//    string_view pattern = argv[2];
//
//    azgra::Stopwatch stopwatch;
//
//    auto bfStopwatch = stopwatch.start_new_stopwatch();
//    auto bruteForce = brute_force(text, pattern);
//    stopwatch.stop(bfStopwatch);
//
//    auto hrsplStopwatch = stopwatch.start_new_stopwatch();
//    auto hrspl = horspool(text, pattern);
//    stopwatch.stop(hrsplStopwatch);
//
//    auto kmpStopwatch = stopwatch.start_new_stopwatch();
//    auto kmp = knuth_morris_pratt(text, pattern);
//    stopwatch.stop(kmpStopwatch);
//
//    fprintf(stdout, "Brute force: %lu  [%.5f ms]\n", bruteForce.size(), stopwatch.elapsed_milliseconds(bfStopwatch));
//    for (const size_t index : bruteForce)
//    {
//        fprintf(stdout, "%lu, ", index);
//    }
//    fprintf(stdout, "\n");
//
//    fprintf(stdout, "Horsepool: %lu  [%.5f ms]\n", hrspl.size(), stopwatch.elapsed_milliseconds(hrsplStopwatch));
//    for (const size_t index : hrspl)
//    {
//        fprintf(stdout, "%lu, ", index);
//    }
//    fprintf(stdout, "\n");
//
//    fprintf(stdout, "Knuth-Morris-Pratt: %lu  [%.5f ms]\n", kmp.size(), stopwatch.elapsed_milliseconds(kmpStopwatch));
//    for (const size_t index : kmp)
//    {
//        fprintf(stdout, "%lu, ", index);
//    }
//    fprintf(stdout, "\n");

    if (argc < 4)
    {
        fprintf(stdout, "[automata word] [# of errors] [test word]\n");
        return 1;
    }
    const char *automataWord = argv[1];//"survey";
    const char *test = argv[3];//"survey";
    size_t maxErrorCount = static_cast<size_t >(atol(argv[2]));
    auto automata = generate_gnfa_for_word(automataWord, maxErrorCount);
    auto acceptResult = automata.accept(test);
    fprintf(stdout, "Automata for: '%s' %s the word '%s' with # of errors: %i\n", automataWord, acceptResult.first ? "accepted" :
                                                                                                "refused", test, acceptResult.second);
    return 0;
}