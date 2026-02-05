#include <bits/stdc++.h>

using namespace std;

map<char, vector<string>> grammar;
map<char, set<char>> firstSet;
map<char, set<char>> followSet;

bool isNonTerminal(char c)
{
    return (c >= 'A' && c <= 'Z');
}

// Compute FIRST set for a non-terminal
void computeFirst(char nonTerminal)
{
    if(firstSet.find(nonTerminal) != firstSet.end() && !firstSet[nonTerminal].empty())
        return;

    for(string &production : grammar[nonTerminal])
    {
        for(int i = 0; i < production.length(); i++)
        {
            char symbol = production[i];
            
            if(!isNonTerminal(symbol))
            {
                // Terminal or epsilon (@)
                firstSet[nonTerminal].insert(symbol);
                break;
            }
            else
            {
                // Non-terminal: get its FIRST set
                computeFirst(symbol);
                
                // Add all non-epsilon symbols from FIRST(symbol)
                for(char c : firstSet[symbol])
                {
                    if(c != '@')
                        firstSet[nonTerminal].insert(c);
                }
                
                // If epsilon is not in FIRST(symbol), stop
                if(firstSet[symbol].find('@') == firstSet[symbol].end())
                    break;
                
                // If we reached the end and all had epsilon, add epsilon
                if(i == production.length() - 1)
                    firstSet[nonTerminal].insert('@');
            }
        }
    }
}

// Compute FOLLOW set for all non-terminals
void computeFollow(char startSymbol)
{
    // Rule 1: Add $ to FOLLOW of start symbol
    followSet[startSymbol].insert('$');
    
    bool changed = true;
    while(changed)
    {
        changed = false;
        
        for(auto &entry : grammar)
        {
            char lhs = entry.first;
            
            for(string &production : entry.second)
            {
                for(int i = 0; i < production.length(); i++)
                {
                    char symbol = production[i];
                    
                    if(!isNonTerminal(symbol))
                        continue;
                    
                    int prevSize = followSet[symbol].size();
                    
                    // Check what follows this non-terminal
                    bool addFollowOfLhs = true;
                    
                    for(int j = i + 1; j < production.length(); j++)
                    {
                        char nextSymbol = production[j];
                        
                        if(!isNonTerminal(nextSymbol))
                        {
                            // Rule 2: Terminal follows - add it
                            followSet[symbol].insert(nextSymbol);
                            addFollowOfLhs = false;
                            break;
                        }
                        else
                        {
                            // Rule 2: Non-terminal follows - add FIRST(nextSymbol) - epsilon
                            for(char c : firstSet[nextSymbol])
                            {
                                if(c != '@')
                                    followSet[symbol].insert(c);
                            }
                            
                            // If nextSymbol cannot derive epsilon, stop
                            if(firstSet[nextSymbol].find('@') == firstSet[nextSymbol].end())
                            {
                                addFollowOfLhs = false;
                                break;
                            }
                        }
                    }
                    
                    // Rule 3: If at end or all following symbols can derive epsilon
                    if(addFollowOfLhs)
                    {
                        for(char c : followSet[lhs])
                        {
                            followSet[symbol].insert(c);
                        }
                    }
                    
                    if(followSet[symbol].size() > prevSize)
                        changed = true;
                }
            }
        }
    }
}

void printFirstSets()
{
    cout << "\n========== FIRST SETS ==========\n";
    for(auto &entry : firstSet)
    {
        cout << "FIRST(" << entry.first << ") = { ";
        bool first = true;
        for(char c : entry.second)
        {
            if(!first) cout << ", ";
            if(c == '@')
                cout << "ε";
            else
                cout << c;
            first = false;
        }
        cout << " }\n";
    }
}

void printFollowSets()
{
    cout << "\n========== FOLLOW SETS ==========\n";
    for(auto &entry : followSet)
    {
        cout << "FOLLOW(" << entry.first << ") = { ";
        bool first = true;
        for(char c : entry.second)
        {
            if(!first) cout << ", ";
            cout << c;
            first = false;
        }
        cout << " }\n";
    }
}

int main()
{
    // Define the grammar
    // S -> ABC | D
    // A -> a | @
    // B -> b | @
    // C -> (S) | c
    // D -> AC
    
    grammar['S'] = {"ABC", "D"};
    grammar['A'] = {"a", "@"};
    grammar['B'] = {"b", "@"};
    grammar['C'] = {"(S)", "c"};
    grammar['D'] = {"AC"};

    cout << "========== GRAMMAR ==========\n";
    for(auto &entry : grammar)
    {
        cout << entry.first << " -> ";
        for(int i = 0; i < entry.second.size(); i++)
        {
            if(i > 0) cout << " | ";
            string prod = entry.second[i];
            for(char c : prod)
            {
                if(c == '@')
                    cout << "ε";
                else
                    cout << c;
            }
        }
        cout << endl;
    }

    // Compute FIRST sets for all non-terminals
    for(auto &entry : grammar)
    {
        computeFirst(entry.first);
    }

    // Compute FOLLOW sets (S is the start symbol)
    computeFollow('S');

    printFirstSets();
    printFollowSets();

    return 0;
}