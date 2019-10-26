#include "regex.h"


namespace regex
{
    inline bool is_char(int c)
    { return (isalnum(c) != 0); }

    void RegularExpression::simple_parse(azgra::string::SmartStringView<char> exp)
    {
        always_assert(isalnum(exp[0]));

        size_t i = 0;
        size_t len = exp.length();
        PairRegEx *lastOp = nullptr;
        while (i < len)
        {
            char c = exp[i++];
            always_assert(is_char(c) != 0 || c == OrSymbol || exp[c] == IterSymbol);

            if (i - 1 == 0)
            {
                always_assert(is_char(c));
                auto *reS = new RegExSymbol();
                reS->symbol = c;
                tokens.push_back(reS);
                continue;
            }

            if (c == OrSymbol)
            {
                always_assert(i > 0 && i < len - 1);
                lastOp = new RegExOr();
                lastOp->a = tokens[tokens.size() - 1];
                continue;
            }
            else if (c == IterSymbol)
            {
                always_assert(i > 0);
                auto *iterRegex = new RegExIter();
                iterRegex->innerEx = tokens[tokens.size() - 1];
                tokens.push_back(iterRegex);
            }
            else if (is_char(c))
            {
                auto *symbol = new RegExSymbol(c);

                if (lastOp)
                {
                    tokens.push_back(symbol);
                    lastOp->b = symbol;
                    tokens.push_back(lastOp);
                    lastOp = nullptr;
                }
                else
                {
                    // prev can be iter or char
                    RegEx *prevToken = tokens[tokens.size() - 1];
                    auto *prevSymbol = dynamic_cast<RegExSymbol *>(prevToken);
                    auto *prevIt = dynamic_cast<RegExIter *>(prevToken);
                    // if prev is symbol
                    if (prevSymbol || prevIt)
                    {
                        auto *conc = new RegExConcat();
                        conc->a = prevToken;
                        conc->b = symbol;
                        tokens.push_back(symbol);
                        tokens.push_back(conc);
                        continue;
                    }
                    always_assert(false && "Wrong path");
                }
            }

        }

    }
}