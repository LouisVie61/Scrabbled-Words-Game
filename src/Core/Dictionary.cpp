#include "Dictionary.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

Dictionary::Dictionary() {}
Dictionary::~Dictionary() {words.clear();}

bool Dictionary::loadFromFile(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening dictionary file: " << filename << std::endl;
        return false;
    }

    string word;
    int wordCount = 0;

    while (getline(file, word)) {
        word.erase(word.begin(), find_if(word.begin(), word.end(), [](unsigned char ch) {
            return !isspace(ch);
        }));

        if (!word.empty()) {
            transform(word.begin(), word.end(), word.begin(), ::toupper);

            if (all_of(word.begin(), word.end(), ::isalpha)) {
                words.insert(word);
                ++wordCount;
            }
        }
    }

    file.close();
    cout << "Loaded " << wordCount << " words from dictionary.\n";
    return !words.empty();
}

bool Dictionary::isValidWord(const string& word) const {
    if (word.empty()) return false;

    string upperWord = word;
    transform(upperWord.begin(), upperWord.end(), upperWord.begin(), ::toupper);

    return words.find(upperWord) != words.end();
}

vector<string> Dictionary::getSuggestions(const string& partial) const {
    vector<string> suggestions;
    string upperPartial = partial;
    transform(upperPartial.begin(), upperPartial.end(), upperPartial.begin(), ::toupper);

    for (const auto& word : words) {
        if (word.length() >= upperPartial.length() && 
            word.substr(0, upperPartial.length()) == upperPartial) {
            suggestions.push_back(word);
            
            // Limit suggestions
            if (suggestions.size() >= 50) {
                break;
            }
        }
    }

    // Sort suggestions by length (shorter words first)
    sort(suggestions.begin(), suggestions.end(), [](const std::string& a, const std::string& b) {
                  if (a.length() == b.length()) {
                      return a < b;
                  }
                  return a.length() < b.length();
              });

    return suggestions;
}

size_t Dictionary::getWordCount() const {
    return words.size();
}