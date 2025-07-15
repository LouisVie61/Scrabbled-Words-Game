#pragma once
#include <unordered_map>

class Tile {
private:
    char letter;
    int points;
    bool isBlank;
    
    // faster look-up
    static const std::unordered_map<char, int> LETTER_POINTS;
    
public:
    Tile();
    Tile(char letter);
    Tile(char letter, int points);
    ~Tile();
    
    char getLetter() const;
    int getPoints() const;
    bool getIsBlank() const;
    void setLetter(char letter);
    void setAsBlank();
    
    static int getPointsForLetter(char letter);
    
    Tile(const Tile& other);
    Tile& operator=(const Tile& other);
};