#include "Game.hpp"
#include "../UI/GameRenderer.hpp"
#include <iostream>
#include <algorithm>
#include <random>

Game::Game() : gameState(GameState::MENU), gameMode(GameMode::HUMAN_VS_HUMAN),
               currentPlayerIndex(0), gameOver(false), consecutivePasses(0),
               window(nullptr), renderer(nullptr), isRunning(false) {
}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow("Scrabble Word Game", 
                             WINDOW_WIDTH, WINDOW_HEIGHT, 
                             SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    gameRenderer = std::make_unique<GameRenderer>(renderer, window);


    // Load dictionary
    if (!loadDictionary("src/Constant/word_bank.txt")) {
        std::cerr << "Warning: Could not load dictionary file" << std::endl;
    }
    
    isRunning = true;
    return true;
}

void Game::cleanup() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}

bool Game::setupGame(GameMode mode, const std::string& player1Name, 
                     const std::string& player2Name) {
    gameMode = mode;
    
    // Initialize players based on game mode
    switch (mode) {
        case GameMode::HUMAN_VS_HUMAN:
            player1 = Player(player1Name, PlayerType::HUMAN);
            player2 = Player(player2Name, PlayerType::HUMAN);
            break;
            
        case GameMode::HUMAN_VS_AI:
            player1 = Player(player1Name, PlayerType::HUMAN);
            player2 = Player(player2Name, PlayerType::AI_MEDIUM);
            break;
            
        case GameMode::AI_VS_AI:
            player1 = Player(player1Name, PlayerType::AI_EASY);
            player2 = Player(player2Name, PlayerType::AI_HARD);
            break;
    }
    
    // Initialize game components
    initializeTileBag();
    fillPlayerRacks();
    
    currentPlayerIndex = 0;
    gameOver = false;
    consecutivePasses = 0;
    gameState = GameState::PLAYING;
    
    return true;
}

bool Game::loadDictionary(const std::string& filename) {
    return dictionary.loadFromFile(filename);
}

void Game::initializeTileBag() {
    // Clear existing tiles
    while (!tileBag.empty()) {
        tileBag.pop();
    }
    
    // Standard Scrabble tile distribution
    struct TileInfo { char letter; int count; };
    
    std::vector<TileInfo> tileDistribution = {
        {'A', 9}, {'B', 2}, {'C', 2}, {'D', 4}, {'E', 12}, {'F', 2},
        {'G', 3}, {'H', 2}, {'I', 9}, {'J', 1}, {'K', 1}, {'L', 4},
        {'M', 2}, {'N', 6}, {'O', 8}, {'P', 2}, {'Q', 1}, {'R', 6},
        {'S', 4}, {'T', 6}, {'U', 4}, {'V', 2}, {'W', 2}, {'X', 1},
        {'Y', 2}, {'Z', 1}, {' ', 2} // 2 blank tiles
    };
    
    // Add tiles to bag
    for (const auto& tileInfo : tileDistribution) {
        for (int i = 0; i < tileInfo.count; ++i) {
            if (tileInfo.letter == ' ') {
                tileBag.push(Tile()); // Blank tile
            } else {
                tileBag.push(Tile(tileInfo.letter)); // Auto-assigns points
            }
        }
    }
    
    // Shuffle the tile bag
    std::vector<Tile> tempTiles;
    while (!tileBag.empty()) {
        tempTiles.push_back(tileBag.front());
        tileBag.pop();
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(tempTiles.begin(), tempTiles.end(), gen);
    
    for (const auto& tile : tempTiles) {
        tileBag.push(tile);
    }
}

bool Game::drawTilesForPlayer(Player& player, int count) {
    int drawn = 0;
    while (drawn < count && !tileBag.empty() && player.hasRoomInRack()) {
        player.addTileToRack(tileBag.front());
        tileBag.pop();
        drawn++;
    }
    return drawn == count;
}

void Game::fillPlayerRacks() {
    // Fill both players' racks to 7 tiles
    while (player1.getRackSize() < 7 && !tileBag.empty()) {
        drawTilesForPlayer(player1, 1);
    }
    while (player2.getRackSize() < 7 && !tileBag.empty()) {
        drawTilesForPlayer(player2, 1);
    }
}

void Game::run() {
    while (isRunning) {
        handleEvents();
        
        // Handle AI moves
        if (gameState == GameState::PLAYING && getCurrentPlayer().isAI()) {
            // Simple AI: skip turn for now (implement AI logic later)
            skipTurn();
        }
        
        render();
        SDL_Delay(16); // ~60 FPS
    }
}

bool Game::isGameRunning() const {
    return isRunning;
}

void Game::startNewGame() {
    // Reset board and game state
    board.clear();
    setupGame(gameMode, player1.getName(), player2.getName());
}

void Game::endGame() {
    gameState = GameState::GAME_OVER;
    gameOver = true;
    
    // Calculate final scores (subtract remaining tile points)
    for (const auto& tile : player1.getRack()) {
        player1.subtractScore(tile.getPoints());
    }
    for (const auto& tile : player2.getRack()) {
        player2.subtractScore(tile.getPoints());
    }
    
    std::cout << "Game Over!" << std::endl;
    std::cout << player1.getName() << ": " << player1.getScore() << std::endl;
    std::cout << player2.getName() << ": " << player2.getScore() << std::endl;
}

const Player& Game::getCurrentPlayer() const {
    return (currentPlayerIndex == 0) ? player1 : player2;
}

const Player& Game::getOtherPlayer() const {
    return (currentPlayerIndex == 0) ? player2 : player1;
}

Player& Game::getCurrentPlayer() {
    return (currentPlayerIndex == 0) ? player1 : player2;
}

Player& Game::getOtherPlayer() {
    return (currentPlayerIndex == 0) ? player2 : player1;
}

void Game::switchTurn() {
    currentPlayerIndex = (currentPlayerIndex == 0) ? 1 : 0;
}

bool Game::checkGameEnd() {
    // Game ends if:
    // 1. One player uses all tiles and tile bag is empty
    // 2. Consecutive passes reach limit
    
    if (consecutivePasses >= MAX_CONSECUTIVE_PASSES) {
        endGame();
        return true;
    }
    
    if (tileBag.empty() && 
        (player1.getRackSize() == 0 || player2.getRackSize() == 0)) {
        endGame();
        return true;
    }
    
    return false;
}

bool Game::playWord(const std::string& word, int startRow, int startCol, 
                    const std::string& direction, const std::vector<int>& tileIndices) {
    // Validate word
    if (!isValidWord(word)) {
        return false;
    }
    
    // Create tiles vector from player's rack
    std::vector<Tile> tiles;
    for (int index : tileIndices) {
        Tile* tile = getCurrentPlayer().getTileFromRack(index);
        if (!tile) return false;
        tiles.push_back(*tile);
    }
    
    // Validate placement
    if (!board.isValidPlacement(startRow, startCol, tiles, direction)) {
        return false;
    }
    
    // Place tiles on board
    bool isHorizontal = (direction == "HORIZONTAL" || direction == "H");
    for (size_t i = 0; i < tiles.size(); ++i) {
        int row = isHorizontal ? startRow : startRow + static_cast<int>(i);
        int col = isHorizontal ? startCol + static_cast<int>(i) : startCol;
        board.placeTile(row, col, tiles[i]);
    }
    
    // Calculate and add score
    int score = calculateWordScore(word, startRow, startCol, direction);
    getCurrentPlayer().addScore(score);
    
    // Remove used tiles from player's rack (in reverse order to maintain indices)
    auto sortedIndices = tileIndices;
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());
    for (int index : sortedIndices) {
        getCurrentPlayer().removeTileFromRack(index);
    }
    
    // Draw new tiles
    drawTilesForPlayer(getCurrentPlayer(), static_cast<int>(tileIndices.size()));
    
    consecutivePasses = 0; // Reset pass counter
    switchTurn();
    
    return true;
}

bool Game::exchangeTiles(const std::vector<int>& tileIndices) {
    if (tileBag.size() < tileIndices.size()) {
        return false; // Not enough tiles in bag
    }
    
    // Remove tiles from rack and add to bag
    std::vector<Tile> exchangedTiles;
    auto sortedIndices = tileIndices;
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());
    
    for (int index : sortedIndices) {
        Tile* tile = getCurrentPlayer().getTileFromRack(index);
        if (tile) {
            exchangedTiles.push_back(*tile);
            getCurrentPlayer().removeTileFromRack(index);
        }
    }
    
    // Draw new tiles
    drawTilesForPlayer(getCurrentPlayer(), static_cast<int>(tileIndices.size()));
    
    // Add exchanged tiles back to bag (should shuffle)
    // For simplicity, we'll skip the shuffle for now
    
    consecutivePasses = 0; // Reset pass counter
    switchTurn();
    
    return true;
}

void Game::skipTurn() {
    consecutivePasses++;
    switchTurn();
    checkGameEnd();
}

bool Game::isValidWord(const std::string& word) const {
    return dictionary.isValidWord(word);
}

int Game::calculateWordScore(const std::string& word, int startRow, int startCol, 
                           const std::string& direction) const {
    return board.calculateWordScore(startRow, startCol, word, direction);
}

void Game::render() {
    gameRenderer->clear();
    
    switch (gameState) {
        case GameState::MENU:
            gameRenderer->renderMenu();
            break;
        case GameState::PLAYING:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderScores(player1, player2);
            gameRenderer->renderGameState(*this);
            break;
        case GameState::GAME_OVER:
            gameRenderer->renderBoard(board);
            gameRenderer->renderScores(player1, player2);
            gameRenderer->renderGameOver(player1, player2);
            break;
        case GameState::PAUSED:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderScores(player1, player2);
            gameRenderer->renderPauseScreen();
            break;
    }
    
    gameRenderer->present();
}

// Add these methods to Game class:

void Game::printGameState() const {
    std::cout << "\n=== GAME STATE ===" << std::endl;
    std::cout << "Current Player: " << getCurrentPlayerIndex() << std::endl;
    std::cout << "Player 1 (" << player1.getName() << "): Score = " << player1.getScore() << std::endl;
    std::cout << "Player 2 (" << player2.getName() << "): Score = " << player2.getScore() << std::endl;
    std::cout << "Tiles in bag: " << tileBag.size() << std::endl;
    std::cout << "Consecutive passes: " << consecutivePasses << std::endl;
    
    // Use direct access instead of getCurrentPlayer()
    const auto& rack = (currentPlayerIndex == 0) ? player1.getRack() : player2.getRack();
    std::cout << "Current player's rack: ";
    for (const auto& tile : rack) {
        std::cout << tile.getLetter() << "(" << tile.getPoints() << ") ";
    }
    std::cout << std::endl;
}

void Game::testWordValidation() {
    std::vector<std::string> testWords = {"HELLO", "WORLD", "SCRABBLE", "INVALID", "TEST"};
    
    std::cout << "\n=== WORD VALIDATION TEST ===" << std::endl;
    for (const std::string& word : testWords) {
        bool valid = isValidWord(word);
        std::cout << "Word: " << word << " - " << (valid ? "VALID" : "INVALID") << std::endl;
    }
}

void Game::placeTestWord() {
    // Place a test word on the board
    std::vector<Tile> testTiles = {
        Tile('H', 4),
        Tile('E', 1),
        Tile('L', 1),
        Tile('L', 1),
        Tile('O', 1)
    };
    
    // Place HELLO horizontally starting at center
    int startRow = 7, startCol = 6;
    for (size_t i = 0; i < testTiles.size(); i++) {
        board.placeTile(startRow, startCol + static_cast<int>(i), testTiles[i]);
    }
    
    std::cout << "Placed test word 'HELLO' on board!" << std::endl;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                handleMouseClick(event.button.x, event.button.y);
                break;
            case SDL_EVENT_KEY_DOWN:
                handleKeyPress(event.key.key);
                break;
        }
    }
}

bool Game::handleMouseClick(int x, int y) {
    // TODO: Implement mouse click handling
    // Board interaction, tile selection, etc.
    return true;
}

bool Game::handleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_ESCAPE:
            if (gameState == GameState::PLAYING) {
                gameState = GameState::PAUSED;
            } else {
                isRunning = false;
            }
            break;
        case SDLK_SPACE:
            if (gameState == GameState::PLAYING) {
                skipTurn();
            }
            break;
        case SDLK_R:
            // Reset board
            board.clear();
            std::cout << "Board cleared!" << std::endl;
            break;
            
        case SDLK_T:
            // Switch turns manually
            switchTurn();
            std::cout << "Switched to player: " << getCurrentPlayerIndex() << std::endl;
            break;
            
        case SDLK_D:
            // Draw tiles for current player
            drawTilesForPlayer(getCurrentPlayer(), 1);
            std::cout << "Drew tile for player " << getCurrentPlayerIndex() << std::endl;
            break;
            
        case SDLK_P:
            // Print game state
            printGameState();
            break;
            
        case SDLK_W:
            // Test word validation
            testWordValidation();
            break;
        case SDLK_1:
            placeTestWord();
            break;
        case SDLK_2:
            // Add current player's score
            getCurrentPlayer().addScore(10);
            std::cout << "Added 10 points to current player" << std::endl;
            break;
        case SDLK_3:
            // Fill current player's rack with test tiles
            getCurrentPlayer().clearRack();
            for (char c = 'A'; c <= 'G'; c++) {
                getCurrentPlayer().addTileToRack(Tile(c, 1));
            }
            std::cout << "Filled rack with test tiles A-G" << std::endl;
            break;
        default:
            break;
    }
    return true;
}

// Getters
const Board& Game::getBoard() const {
    return board;
}

GameState Game::getGameState() const {
    return gameState;
}

GameMode Game::getGameMode() const {
    return gameMode;
}

int Game::getCurrentPlayerIndex() const {
    return currentPlayerIndex;
}