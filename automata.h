#pragma once

#include <algorithm>
#include <vector>
#include <string_view>
#include <set>

struct Symbol
{
    char c;
    bool epsilon;
    bool sigma;

    static Symbol Char(const char _c)
    {
        Symbol s = {};
        s.c = _c;
        s.epsilon = false;
        s.sigma = false;
        return s;
    }

    static Symbol Epsilon()
    {
        Symbol s = {};
        s.c = '\0';
        s.epsilon = true;
        s.sigma = false;
        return s;
    }

    static Symbol Sigma()
    {
        Symbol s = {};
        s.c = '\0';
        s.epsilon = false;
        s.sigma = true;
        return s;
    }
};

struct Transition
{
    size_t from;
    size_t to;
    Symbol s;

    Transition() = default;

    Transition(size_t _from, Symbol _s, size_t _to) : from(_from), to(_to), s(_s)
    {}
};

struct GNFA
{
    std::vector<size_t> initialStates;
    std::vector<size_t> finalStates;
    std::vector<Transition> transitions;

    std::set<size_t> get_epsilon_connected_states(const std::vector<size_t> &states)
    {
        std::vector<size_t> connectedStates;
        for (const size_t &state : states)
        {
            for (const Transition &t : transitions)
            {
                if ((t.from == state) && t.s.epsilon)
                {
                    connectedStates.push_back(t.to);
                }
            }
        }

        auto connectedToConnected = get_epsilon_connected_states(connectedStates);
        connectedStates.insert(connectedStates.end(), connectedToConnected.begin(), connectedToConnected.end());
        std::set<size_t> result(connectedStates.begin(), connectedStates.end());
        return result;
    }


    std::set<size_t> get_next_states(const std::set<size_t> &currentStates, const char &c)
    {
        std::vector<size_t> nextStates;

        for (const Transition &t : transitions)
        {
            if (t.s.sigma || t.s.c == c)
            {
                nextStates.push_back(t.to);
            }
        }
        auto connected = get_epsilon_connected_states(nextStates);
        nextStates.insert(nextStates.end(), connected.begin(), connected.end());
        std::set<size_t> result(nextStates.begin(), nextStates.end());
        return result;
    }


    bool accept(const std::string_view &word)
    {
        // TODO: Add epsilon connected states to inial states.
        std::set<size_t> currentStates(initialStates.begin(), initialStates.end());
        for (const char &c : word)
        {
            currentStates = get_next_states(currentStates, c);
            if (currentStates.empty())
            {
                fprintf(stdout, "Empty next states!\n");
            }
        }

        for (const size_t &s : currentStates)
        {
            if (std::find(finalStates.begin(), finalStates.end(), s) != finalStates.end())
            {
                return true;
            }
        }
        return false;
    }
};

inline GNFA generate_gnfa_for_word(const std::string_view &word, const size_t maxErrorCount)
{
    const size_t N = word.length();
    const size_t colCount = N + 1;
    const size_t rowCount = maxErrorCount + 1;

    GNFA result = {};
    result.initialStates.push_back(0);

    for (size_t row = 0; row < rowCount; ++row)
    {
        for (size_t col = 0; col < colCount; ++col)
        {
            size_t currentState = (row * colCount) + col;
            // Final state in row.
            if (col == (colCount - 1))
            {
                result.finalStates.push_back(currentState);
                Transition downInsertion(currentState, Symbol::Sigma(), currentState + colCount);
                result.transitions.push_back(downInsertion);
            }
            else
            {
                Transition leftToRightMatch(currentState, Symbol::Char(word[col]), currentState + 1);
                Transition downInsertion(currentState, Symbol::Sigma(), currentState + colCount);
                Transition diagonalDel(currentState, Symbol::Epsilon(), currentState + colCount + 1);
                Transition diagonalReplace(currentState, Symbol::Sigma(), currentState + colCount + 1);
                result.transitions.push_back(leftToRightMatch);
                result.transitions.push_back(downInsertion);
                result.transitions.push_back(diagonalDel);
                result.transitions.push_back(diagonalReplace);
            }

        }
    }

    return result;
}

/*
 * Slovo delky N
 * kazdy radek ma N + 1 stavu, cislovat po radcich
 * pocet chyb k, celkovy pocet stavu = (N+1)*(k+1)
 * */