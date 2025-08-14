#include "Tile.hpp"
#include <cctype>
using namespace std;

const unordered_map<char, int> Tile::LETTER_POINTS = {
    // 1 point letters (12 letters)
    {'A', 1}, {'E', 1}, {'I', 1}, {'O', 1}, {'U', 1},
    {'L', 1}, {'N', 1}, {'S', 1}, {'T', 1}, {'R', 1},
    
    // 2 point letters (2 letters)
    {'D', 2}, {'G', 2},
    
    // 3 point letters (4 letters)
    {'B', 3}, {'C', 3}, {'M', 3}, {'P', 3},
    
    // 4 point letters (5 letters)
    {'F', 4}, {'H', 4}, {'V', 4}, {'W', 4}, {'Y', 4},
    
    // 5 point letters (1 letter)
    {'K', 5},
    
    // 8 point letters (2 letters)
    {'J', 8}, {'X', 8},
    
    // 10 point letters (2 letters)
    {'Q', 10}, {'Z', 10}
};

Tile::Tile() : letter(' '), points(0), isBlank(true) {}
Tile::Tile(char letter) : letter(toupper(letter)), points(getPointsForLetter(letter)), isBlank(false) {
    points = getPointsForLetter(letter);
}
Tile::Tile(char letter, int points) : letter(std::toupper(letter)), points(points), isBlank(false) {}
Tile::Tile(const Tile& other) : letter(other.letter), points(other.points), isBlank(other.isBlank) {}
Tile& Tile::operator=(const Tile& other) {
    if (this != &other) {
        letter = other.letter;
        points = other.points;
        isBlank = other.isBlank;
    }
    return *this;
}
Tile::~Tile() {}

char Tile::getLetter() const {
    return letter;
}
int Tile::getPoints() const {
    return points;
}
bool Tile::getIsBlank() const {
    return isBlank;
}

void Tile::setLetter(char newLetter) {
    this->letter = std::toupper(newLetter);
    
    if (isBlank && newLetter != ' ') {
        isBlank = false;
        points = 0;
    } else if (!isBlank) {
        points = getPointsForLetter(newLetter);
    }
}
void Tile::setAsBlank() {
    letter = ' ';
    points = 0;
    isBlank = true;
}
int Tile::getPointsForLetter(char letter) {
    auto it = LETTER_POINTS.find(std::toupper(letter));
    if (it != LETTER_POINTS.end()) {
        return it->second;
    }
    return 0;
}