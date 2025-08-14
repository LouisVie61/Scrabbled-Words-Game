#pragma once
#include "Tile.hpp"
#include <vector>
#include <string>
using namespace std;

enum class PlayerType {
    HUMAN,
    AI_EASY,
    AI_MEDIUM,
    AI_HARD
};

class Player {
private:
    string name;
    int score;
    vector<Tile> rack;
    PlayerType playerType;
    static const int RACK_SIZE = 7;
    
public:
    Player();
    Player(const string& name, PlayerType type = PlayerType::HUMAN);
    ~Player();
    
    // Getters
    const std::string& getName() const;
    int getScore() const;
    const std::vector<Tile>& getRack() const;
    PlayerType getPlayerType() const;
    bool isAI() const;
    
    // Score management
    void addScore(int points);
    void subtractScore(int points);
    void resetScore();
    
    // Rack management
    bool addTileToRack(const Tile& tile);
    bool removeTileFromRack(int index);
    Tile* getTileFromRack(int index);
    bool hasRoomInRack() const;
    int getRackSize() const;
    void clearRack();
    
    // Game actions
    void shuffleRack();
    bool canFormWord(const string& word) const;
    std::vector<int> findTilesForWord(const string& word) const;
    
    // AI-specific methods
    bool makeAIMove();
};