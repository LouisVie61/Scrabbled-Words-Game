#include "Board.hpp"
using namespace std;

Board::Board() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            tiles[i][j] = nullptr;
        }
    }
}

Board::~Board() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            delete tiles[i][j];
        }
    }
}

bool Board::placeTile(int row, int col, const Tile& tile) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false;
    }
    if (tiles[row][col] != nullptr) {
        return false;
    }

    tiles[row][col] = new Tile(tile);
    return true;
}

bool Board::removeTile(int row, int col) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false;
    }
    
    if (tiles[row][col] != nullptr) {
        delete tiles[row][col];
        tiles[row][col] = nullptr;
        return true;
    }
    
    return false;
}

Tile* Board::getTile(int row, int col) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return nullptr;
    }
    return tiles[row][col];
}

SpecialSquare Board::getSpecialSquare(int row, int col) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return SpecialSquare::NORMAL;
    }
    return specialSquares[row][col];
}

bool Board::isValidPlacement(int row, int col, const vector<Tile>& tilesToPlace, 
                             const string& direction) const {
    
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return false;

    bool isHorizontal = (direction == "HORIZONTAL" || direction == "H");
    bool isVertical = (direction == "VERTICAL" || direction == "V");
    
    if (!isHorizontal && !isVertical) return false;

    for (size_t i = 0; i < tilesToPlace.size(); i++) {
        int curRow = isVertical ? row + static_cast<int>(i) : row;
        int curCol = isHorizontal ? col + static_cast<int>(i) : col;

        if (curRow >= BOARD_SIZE || curCol >= BOARD_SIZE) return false;
        if (tiles[curRow][curCol] != nullptr) return false;
    }

    if (isEmpty()) {
        bool isHorizontal = (direction == "HORIZONTAL" || direction == "H");
        
        for (size_t i = 0; i < tilesToPlace.size(); i++) {
            int curRow = isVertical ? row + static_cast<int>(i) : row;
            int curCol = isHorizontal ? col + static_cast<int>(i) : col;

            if (curRow == 7 && curCol == 7) {
                return true;
            }
        }
        return false;
    }

    bool connectsToExisting = false;
    for (size_t i = 0; i < tilesToPlace.size(); i++) {
        int curRow = isVertical ? row + static_cast<int>(i) : row;
        int curCol = isHorizontal ? col + static_cast<int>(i) : col;
        
        int adjacentPositions[][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int j = 0; j < 4; j++) {
            int adjRow = curRow + adjacentPositions[j][0];
            int adjCol = curCol + adjacentPositions[j][1];
            
            if (adjRow >= 0 && adjRow < BOARD_SIZE && adjCol >= 0 && adjCol < BOARD_SIZE) {
                if (tiles[adjRow][adjCol] != nullptr) {
                    connectsToExisting = true;
                    break;
                }
            }
        }
        if (connectsToExisting) break;
    }

    return connectsToExisting;
}

int Board::calculateWordScore(int row, int col, const string& word, 
                            const string& direction) const {
    if (word.empty()) return 0;
    
    bool isHorizontal = (direction == "HORIZONTAL" || direction == "H");
    int score = 0;
    int wordMultiplier = 1;
    
    for (size_t i = 0; i < word.length(); i++) {
        int curRow = isHorizontal ? row : row + static_cast<int>(i);
        int curCol = isHorizontal ? col + static_cast<int>(i) : col;
        
        int letterPoints = Tile::getPointsForLetter(word[i]);
        
        if (tiles[curRow][curCol] == nullptr) {
            SpecialSquare special = specialSquares[curRow][curCol];
            
            switch (special) {
                case SpecialSquare::DOUBLE_LETTER:
                    letterPoints *= 2;
                    break;
                case SpecialSquare::TRIPLE_LETTER:
                    letterPoints *= 3;
                    break;
                case SpecialSquare::DOUBLE_WORD:
                case SpecialSquare::CENTER:
                    wordMultiplier *= 2;
                    break;
                case SpecialSquare::TRIPLE_WORD:
                    wordMultiplier *= 3;
                    break;
                default:
                    break;
            }
        }
        
        score += letterPoints;
    }
    
    return score * wordMultiplier;
}

bool Board::isEmpty() const {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (tiles[row][col] != nullptr) {
                return false;
            }
        }
    }
    return true;
}

void Board::clear() {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (tiles[row][col] != nullptr) {
                delete tiles[row][col];
                tiles[row][col] = nullptr;
            }
        }
    }
}