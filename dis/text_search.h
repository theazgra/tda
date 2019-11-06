#pragma once
#include <vector>
#include <string_view>
#include <cstring>

#define CHAR_COUNT 256
using namespace std;

std::vector<size_t> brute_force(const string_view &text, const string_view &pattern);
std::vector<size_t> horspool(const string_view &text, const string_view &pattern);
std::vector<size_t> knuth_morris_pratt(const string_view &text, const string_view &pattern);