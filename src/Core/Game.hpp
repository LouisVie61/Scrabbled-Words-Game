#pragma once
#include "Board.hpp"
#include "Player.hpp"
#include "Dictionary.hpp"
#include "Tile.hpp"
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <SDL3/SDL.h>

class GameRenderer;

struct TilePlacement {
    int row;
    int col;
    const Tile* tile;
};

enum class GameState {
    MENU,
    PLAYING,
    PLACING_TILES,
    VALIDATING_WORD,
    GAME_OVER,
    PAUSED
};

enum class GameMode {
    HUMAN_VS_HUMAN,
    HUMAN_VS_AI,
    AI_VS_AI
};

class Game {
private:
    // Core game components
    int selectedTileIndex; // Track selected tile index for placement
    Board board;
    Player player1;
    Player player2;
    Dictionary dictionary;
    std::queue<Tile> tileBag;
    
    // Game state
    GameState gameState;
    GameMode gameMode;
    int currentPlayerIndex; // 0 = player1, 1 = player2
    bool gameOver;
    int consecutivePasses; // Track consecutive passes/skips
    
    // Add word placement tracking
    std::vector<std::pair<int, int>> currentWordPositions;
    std::string currentWord;
    bool wordInProgress;

    // SDL components
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    
    // UI component
    std::unique_ptr<GameRenderer> gameRenderer;
    
    // Game constants
    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;
    static const int MAX_CONSECUTIVE_PASSES = 6; // End game after 6 passes
    
    // Tile distribution for standard Scrabble
    void initializeTileBag();
    bool drawTilesForPlayer(Player& player, int count = 1);
    void fillPlayerRacks();
    
public:
    Game();
    ~Game();
    
    // Initialization and cleanup
    bool initialize();
    void cleanup();
    
    // Game setup
    bool setupGame(GameMode mode, const std::string& player1Name, 
                   const std::string& player2Name = "Computer");
    bool loadDictionary(const std::string& filename);
    
    // Game flow
    void run();
    bool isGameRunning() const;
    void startNewGame();
    void endGame();
    
    // Turn management
    const Player& getCurrentPlayer() const;
    const Player& getOtherPlayer() const;
    
    Player& getCurrentPlayer();
    Player& getOtherPlayer();
    void switchTurn();
    bool checkGameEnd();
    
    // Game actions
    bool playWord(const std::string& word, int startRow, int startCol, 
                  const std::string& direction, const std::vector<int>& tileIndices);
    bool exchangeTiles(const std::vector<int>& tileIndices);
    void skipTurn();
    bool isValidWord(const std::string& word) const;
    
    // Game logic
    int calculateWordScore(const std::string& word, int startRow, int startCol, 
                          const std::string& direction) const;
    std::vector<std::string> findWordsFormed(int startRow, int startCol, 
                                            const std::string& word, 
                                            const std::string& direction) const;
    
    // Rendering
    void render();
    
    // testing and debugging
    void printHelp() const;
    void printGameState() const;
    void placeTestWord();
    void givePlayerTestTiles();
    void testScoring();
    void testDictionary();
    void resetBoard();

    // Tracking tile
    void selectTileFromRack(int index);
    int getSelectedTileIndex() const;
    void selectNextTile();
    void selectPreviousTile();

    // Game player's actions
    void startWordPlacement();
    int getRackTileIndexFromMouse(int mouseX, int mouseY) const;
    bool placeTileFromRack(int row, int col);
    bool validateCurrentWord();
    void cancelWord();
    void refillPlayerRack();
    std::string buildWordFromPositions() const;
    
    // Event handling
    void handleEvents();
    bool handleMouseClick(int x, int y);
    bool handleKeyPress(SDL_Keycode key);
    
    // Getters for GameRenderer to access game state
    const Board& getBoard() const;
    GameState getGameState() const;
    GameMode getGameMode() const;
    int getCurrentPlayerIndex() const;
    const Player& getPlayer1() const;
    const Player& getPlayer2() const;
    size_t getTileBagSize() const;
    std::vector<TilePlacement> getCurrentWord() const;
};
