#include "Player.hpp"
#include <algorithm>
#include <random>
#include <iostream>

Player::Player() : name("Unknown"), score(0), playerType(PlayerType::HUMAN) {
    rack.reserve(RACK_SIZE);
}

Player::Player(const std::string& name, PlayerType type) 
    : name(name), score(0), playerType(type) {
    rack.reserve(RACK_SIZE);
}

Player::~Player() {
    clearRack();
}

const std::string& Player::getName() const {
    return name;
}

int Player::getScore() const {
    return score;
}

const std::vector<Tile>& Player::getRack() const {
    return rack;
}

PlayerType Player::getPlayerType() const {
    return playerType;
}

bool Player::isAI() const {
    return playerType != PlayerType::HUMAN;
}

// Score management
void Player::addScore(int points) {
    score += points;
}

void Player::subtractScore(int points) {
    score = std::max(0, score - points); // Don't go below 0
}

void Player::resetScore() {
    score = 0;
}

// Rack management
bool Player::addTileToRack(const Tile& tile) {
    if (rack.size() >= RACK_SIZE) {
        return false; // Rack is full
    }
    
    rack.push_back(tile);
    return true;
}

bool Player::removeTileFromRack(int index) {
    if (index < 0 || index >= static_cast<int>(rack.size())) {
        return false; // Invalid index
    }
    
    rack.erase(rack.begin() + index);
    return true;
}

Tile* Player::getTileFromRack(int index) {
    if (index < 0 || index >= static_cast<int>(rack.size())) {
        return nullptr; // Invalid index
    }
    
    return &rack[index];
}

bool Player::hasRoomInRack() const {
    return rack.size() < RACK_SIZE;
}

int Player::getRackSize() const {
    return static_cast<int>(rack.size());
}

void Player::clearRack() {
    rack.clear();
}

// Game actions
void Player::shuffleRack() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(rack.begin(), rack.end(), gen);
}

bool Player::canFormWord(const std::string& word) const {
    if (word.empty()) return false;
    
    // Create a copy of the rack to check against
    std::vector<Tile> availableTiles = rack;
    
    // Check if we have all the letters needed
    for (char letter : word) {
        char upperLetter = std::toupper(letter);
        bool foundTile = false;
        
        for (auto it = availableTiles.begin(); it != availableTiles.end(); ++it) {
            if (it->getLetter() == upperLetter || it->getIsBlank()) {
                availableTiles.erase(it);
                foundTile = true;
                break;
            }
        }
        
        if (!foundTile) {
            return false; // Missing required letter
        }
    }
    
    return true;
}

std::vector<int> Player::findTilesForWord(const std::string& word) const {
    std::vector<int> tileIndices;
    std::vector<bool> used(rack.size(), false);
    
    for (char letter : word) {
        char upperLetter = std::toupper(letter);
        bool foundTile = false;
        
        // First, look for exact letter match
        for (size_t i = 0; i < rack.size(); ++i) {
            if (!used[i] && rack[i].getLetter() == upperLetter) {
                tileIndices.push_back(static_cast<int>(i));
                used[i] = true;
                foundTile = true;
                break;
            }
        }
        
        // If no exact match, look for blank tile
        if (!foundTile) {
            for (size_t i = 0; i < rack.size(); ++i) {
                if (!used[i] && rack[i].getIsBlank()) {
                    tileIndices.push_back(static_cast<int>(i));
                    used[i] = true;
                    foundTile = true;
                    break;
                }
            }
        }
        
        if (!foundTile) {
            return {}; // Return empty vector if word can't be formed
        }
    }
    
    return tileIndices;
}

bool Player::makeAIMove() {
    if (!isAI()) {
        return false;
    }
    
    // TODO: Implement AI decision making
    // - Analyze board
    // - Find best word placement
    // - Calculate scores
    // - Make move
    
    std::cout << "AI " << name << " is thinking..." << std::endl;
    return true;
}