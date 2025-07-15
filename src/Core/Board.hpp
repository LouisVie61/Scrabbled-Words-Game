#pragma once
#include "Tile.hpp"
#include <vector>
#include <string>
using namespace std;

enum class SpecialSquare {
    NORMAL,
    DOUBLE_LETTER,
    TRIPLE_LETTER,
    DOUBLE_WORD,
    TRIPLE_WORD,
    CENTER
};

class Board {
private:
    static const int BOARD_SIZE = 15;
    Tile* tiles[BOARD_SIZE][BOARD_SIZE];

    SpecialSquare specialSquares[BOARD_SIZE][BOARD_SIZE] = {
        {SpecialSquare::TRIPLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_WORD},
        {SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::TRIPLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::CENTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_WORD},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER},
        {SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL},
        {SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_WORD, SpecialSquare::NORMAL},
        {SpecialSquare::TRIPLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_WORD, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::DOUBLE_LETTER, SpecialSquare::NORMAL, SpecialSquare::NORMAL, SpecialSquare::TRIPLE_WORD}
    };
    
public:
    Board();
    ~Board();
    
    // Tile placement
    bool placeTile(int row, int col, const Tile& tile);
    bool removeTile(int row, int col);
    
    // Getters
    Tile* getTile(int row, int col) const;
    SpecialSquare getSpecialSquare(int row, int col) const;
    
    // Word validation
    bool isValidPlacement(int row, int col, const vector<Tile>& tiles, 
                         const string& direction) const;
    
    // Scoring
    int calculateWordScore(int row, int col, const string& word, 
                          const string& direction) const;
    
    // Board state
    bool isEmpty() const;
    void clear();
};