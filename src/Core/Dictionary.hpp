#pragma once
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

class Dictionary {
private:
    std::unordered_set<std::string> words;
    
public:
    Dictionary();
    ~Dictionary();

    bool loadFromFile(const std::string& filename);
    bool isValidWord(const std::string& word) const;
    vector<std::string> getSuggestions(const std::string& partial) const;
    size_t getWordCount() const;
};