#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
using namespace std;

map<char, vector<string>> grammar;
map<char, set<char>> firstSet;
map<char, set<char>> followSet;

bool isNonTerminal(char c)
{
    return (c >= 'A' && c <= 'Z');
}

// First Set
void computeFirst(char nonTerminal)
{
    if (!firstSet[nonTerminal].empty())
        return;

    for (string production : grammar[nonTerminal])
    {
        for (int i = 0; i < production.length(); i++)
        {
            char symbol = production[i];

            if (!isNonTerminal(symbol))
            {
                firstSet[nonTerminal].insert(symbol);
                break;
            }
            else
            {
                computeFirst(symbol);

                for (char c : firstSet[symbol])
                {
                    if (c != '@')
                        firstSet[nonTerminal].insert(c);
                }

                if (firstSet[symbol].find('@') == firstSet[symbol].end())
                    break;

                if (i == production.length() - 1)
                    firstSet[nonTerminal].insert('@');
            }
        }
    }
}

// Follow Set
void computeFollow(char startSymbol)
{
    followSet[startSymbol].insert('$');

    bool changed = true;
    while (changed)
    {
        changed = false;

        for (auto &entry : grammar)
        {
            char lhs = entry.first;

            for (string production : entry.second)
            {
                for (int i = 0; i < production.length(); i++)
                {
                    char symbol = production[i];

                    if (!isNonTerminal(symbol))
                        continue;

                    int prevSize = followSet[symbol].size();
                    bool addFollowLHS = true;

                    for (int j = i + 1; j < production.length(); j++)
                    {
                        char nextSymbol = production[j];

                        if (!isNonTerminal(nextSymbol))
                        {
                            followSet[symbol].insert(nextSymbol);
                            addFollowLHS = false;
                            break;
                        }
                        else
                        {
                            for (char c : firstSet[nextSymbol])
                                if (c != '@')
                                    followSet[symbol].insert(c);

                            if (firstSet[nextSymbol].find('@') == firstSet[nextSymbol].end())
                            {
                                addFollowLHS = false;
                                break;
                            }
                        }
                    }

                    if (addFollowLHS)
                    {
                        for (char c : followSet[lhs])
                            followSet[symbol].insert(c);
                    }

                    if (followSet[symbol].size() > prevSize)
                        changed = true;
                }
            }
        }
    }
}

// Print First and Follow
void printFirst()
{
    cout << "\nFIRST Sets:\n";
    for (auto &entry : firstSet)
    {
        cout << "FIRST(" << entry.first << ") = { ";
        for (char c : entry.second)
            cout << (c == '@' ? "ε" : string(1, c)) << " ";
        cout << "}\n";
    }
}

void printFollow()
{
    cout << "\nFOLLOW Sets:\n";
    for (auto &entry : followSet)
    {
        cout << "FOLLOW(" << entry.first << ") = { ";
        for (char c : entry.second)
            cout << c << " ";
        cout << "}\n";
    }
}

int main()
{
    // Grammar
    grammar['S'] = {"ABC", "D"};
    grammar['A'] = {"a", "@"};
    grammar['B'] = {"b", "@"};
    grammar['C'] = {"(S)", "c"};
    grammar['D'] = {"AC"};
    for (auto &entry : grammar)
        computeFirst(entry.first);

    computeFollow('S');

    printFirst();
    printFollow();

    // LL(1) TABLE
    vector<char> nonTerminals = {'S', 'A', 'B', 'C', 'D'};
    vector<char> terminals = {'a', 'b', 'c', '(', ')', '$'};

    map<char, vector<string>> LL1_table;

    // Initialize table
    for (char nt : nonTerminals)
        LL1_table[nt] = vector<string>(terminals.size(), "");

    // Fill Table
    for (char A : nonTerminals)
    {
        for (string production : grammar[A])
        {
            set<char> firstAlpha;

            // Compute FIRST(production)
            for (int i = 0; i < production.length(); i++)
            {
                char symbol = production[i];

                if (!isNonTerminal(symbol))
                {
                    firstAlpha.insert(symbol);
                    break;
                }
                else
                {
                    for (char c : firstSet[symbol])
                        if (c != '@')
                            firstAlpha.insert(c);

                    if (firstSet[symbol].find('@') == firstSet[symbol].end())
                        break;

                    if (i == production.length() - 1)
                        firstAlpha.insert('@');
                }
            }

            for (char t : firstAlpha)
            {
                if (t != '@')
                {
                    for (int i = 0; i < terminals.size(); i++)
                    {
                        if (terminals[i] == t)
                        {
                            if (LL1_table[A][i] != "")
                            {
                                cout << "\nGrammar is NOT LL(1)\n";
                                return 0;
                            }
                            LL1_table[A][i] = production;
                        }
                    }
                }
                else
                {
                    for (char f : followSet[A])
                    {
                        for (int i = 0; i < terminals.size(); i++)
                        {
                            if (terminals[i] == f)
                            {
                                if (LL1_table[A][i] != "")
                                {
                                    cout << "\nGrammar is NOT LL(1)\n";
                                    return 0;
                                }
                                LL1_table[A][i] = "ε";
                            }
                        }
                    }
                }
            }
        }
    }

    // Print Table
    cout << "\nLL(1) Parsing Table:\n\n   ";
    for (char t : terminals)
        cout << t << "    ";
    cout << "\n";

    for (char nt : nonTerminals)
    {
        cout << nt << "  ";
        for (int i = 0; i < terminals.size(); i++)
        {
            if (LL1_table[nt][i] == "")
                cout << "-    ";
            else
                cout << LL1_table[nt][i] << "    ";
        }
        cout << "\n";
    }

    cout << "\nGrammar IS LL(1)\n";

    return 0;
}
