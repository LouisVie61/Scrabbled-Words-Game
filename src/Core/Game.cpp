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

    player1.shuffleRack();
    player2.shuffleRack();
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

    // Shuffle the new current player's rack
    getCurrentPlayer().shuffleRack();
    std::cout << "🔀 " << getCurrentPlayer().getName() << "'s tiles shuffled!" << std::endl;
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
        case GameState::PLACING_TILES:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderScores(player1, player2);
            gameRenderer->renderGameState(*this);
            break;
        case GameState::VALIDATING_WORD:
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

// Add these methods to Game.cpp:

void Game::printHelp() const {
    std::cout << "\n========== SCRABBLE TESTING CONTROLS ==========" << std::endl;
    std::cout << "H - Show this help" << std::endl;
    std::cout << "P - Print current game state" << std::endl;
    std::cout << "1 - Place test word 'HELLO' on board" << std::endl;
    std::cout << "2 - Give current player test tiles (A-G)" << std::endl;
    std::cout << "3 - Test scoring system" << std::endl;
    std::cout << "4 - Test dictionary (check if words are valid)" << std::endl;
    std::cout << "R - Reset/clear the board" << std::endl;
    std::cout << "T - Switch turns between players" << std::endl;
    std::cout << "SPACE - Skip current player's turn" << std::endl;
    std::cout << "ESC - Pause game / Quit" << std::endl;
    std::cout << "MOUSE CLICK - Place a test tile on the board" << std::endl;
    std::cout << "===============================================" << std::endl;
}

void Game::printGameState() const {
    std::cout << "\n========== CURRENT GAME STATE ==========" << std::endl;
    std::cout << "Current Player: " << (getCurrentPlayerIndex() + 1) 
              << " (" << (currentPlayerIndex == 0 ? player1.getName() : player2.getName()) << ")" << std::endl;
    std::cout << "Player 1 (" << player1.getName() << "): " << player1.getScore() << " points" << std::endl;
    std::cout << "Player 2 (" << player2.getName() << "): " << player2.getScore() << " points" << std::endl;
    std::cout << "Tiles left in bag: " << tileBag.size() << std::endl;
    std::cout << "Consecutive passes: " << consecutivePasses << std::endl;
    
    // Show current player's tiles
    const auto& rack = (currentPlayerIndex == 0) ? player1.getRack() : player2.getRack();
    std::cout << "Current player's tiles: ";
    if (rack.empty()) {
        std::cout << "(no tiles)";
    } else {
        for (const auto& tile : rack) {
            std::cout << tile.getLetter() << "(" << tile.getPoints() << ") ";
        }
    }
    std::cout << "\n========================================" << std::endl;
}

void Game::placeTestWord() {
    std::cout << "\nPlacing test word 'HELLO' on the board..." << std::endl;
    
    // Create tiles for HELLO
    std::vector<Tile> testTiles = {
        Tile('H', 4),  // H is worth 4 points
        Tile('E', 1),  // E is worth 1 point
        Tile('L', 1),  // L is worth 1 point
        Tile('L', 1),  // L is worth 1 point
        Tile('O', 1)   // O is worth 1 point
    };
    
    // Place horizontally starting from center (row 7, col 6)
    int startRow = 7, startCol = 6;
    bool success = true;
    
    for (size_t i = 0; i < testTiles.size(); i++) {
        if (!board.placeTile(startRow, startCol + static_cast<int>(i), testTiles[i])) {
            success = false;
            break;
        }
    }
    
    if (success) {
        std::cout << "Successfully placed 'HELLO' on board!" << std::endl;
        std::cout << "Location: Row " << startRow << ", Columns " << startCol << "-" << (startCol + 4) << std::endl;
        
        // Calculate and show score
        int score = calculateWordScore("HELLO", startRow, startCol, "HORIZONTAL");
        std::cout << "Word score: " << score << " points" << std::endl;
    } else {
        std::cout << "Could not place word (spaces might be occupied)" << std::endl;
    }
}

void Game::givePlayerTestTiles() {
    Player& current = (currentPlayerIndex == 0) ? player1 : player2;
    
    std::cout << "\nGiving test tiles to " << current.getName() << "..." << std::endl;
    
    // Clear current rack and add test tiles
    current.clearRack();
    
    std::vector<Tile> testTiles = {
        Tile('A', 1), Tile('B', 3), Tile('C', 3), Tile('D', 2),
        Tile('E', 1), Tile('F', 4), Tile('G', 2)
    };
    
    for (const auto& tile : testTiles) {
        current.addTileToRack(tile);
    }
    
    std::cout << "Added tiles: A(1) B(3) C(3) D(2) E(1) F(4) G(2)" << std::endl;
    std::cout << current.getName() << " now has " << current.getRack().size() << " tiles" << std::endl;
}

void Game::testScoring() {
    std::cout << "\nTESTING SCORING SYSTEM" << std::endl;
    
    // Test different words and their scores
    std::vector<std::pair<std::string, std::pair<int, int>>> testWords = {
        {"CAT", {7, 5}},      // Row 7, Col 5
        {"DOG", {8, 7}},      // Row 8, Col 7  
        {"GAME", {6, 3}},     // Row 6, Col 3
        {"TEST", {9, 10}}     // Row 9, Col 10
    };
    
    for (const auto& test : testWords) {
        const std::string& word = test.first;
        int row = test.second.first;
        int col = test.second.second;
        
        int score = calculateWordScore(word, row, col, "HORIZONTAL");
        std::cout << "Word: " << word << " at (" << row << "," << col << ") = " << score << " points" << std::endl;
    }
}

void Game::testDictionary() {
    std::cout << "\nTESTING DICTIONARY" << std::endl;
    
    std::vector<std::string> testWords = {
        "HELLO", "WORLD", "SCRABBLE", "COMPUTER", "GAME",
        "INVALID", "XYZZYX", "NOTAWORD", "APPLE", "HOUSE"
    };
    
    std::cout << "Checking words from dictionary..." << std::endl;
    for (const std::string& word : testWords) {
        bool valid = isValidWord(word);
        std::cout << word << " - " << (valid ? "VALID" : "INVALID") << std::endl;
    }
}

void Game::resetBoard() {
    std::cout << "\nClearing the board..." << std::endl;
    board.clear();
    std::cout << "Board cleared! Ready for new tiles." << std::endl;
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
    if (gameState != GameState::PLAYING && gameState != GameState::PLACING_TILES) {
        return false;
    }

    int row, col;
    if (gameRenderer->isPointInBoard(x, y, row, col)) {
        std::cout << "Clicked on board cell: (" << row << ", " << col << ")" << std::endl;
        
        if (gameState == GameState::PLAYING) {
            startWordPlacement();
        }

        return placeTileFromRack(row, col);
    }

    std::cout << "Clicked outside board at: (" << x << ", " << y << ")" << std::endl;
    return false;
}

void Game::startWordPlacement() {
    if (gameState == GameState::PLAYING) {
        currentWordPositions.clear();
        currentWord.clear();
        wordInProgress = true;
        gameState = GameState::PLACING_TILES;
        
        std::cout << "Starting word placement..." << std::endl;
        std::cout << "Click on board cells to place tiles from your rack" << std::endl;
        std::cout << "Press ENTER to confirm word, BACKSPACE to cancel" << std::endl;
        
        // Show current player's rack
        const auto& rack = getCurrentPlayer().getRack();
        std::cout << "Your tiles: ";
        for (size_t i = 0; i < rack.size(); i++) {
            std::cout << "[" << i << "]" << rack[i].getLetter() 
                     << "(" << rack[i].getPoints() << ") ";
        }
        std::cout << std::endl;
    }
}

bool Game::placeTileFromRack(int row, int col) {
    Player& currentPlayer = getCurrentPlayer();
    auto& rack = currentPlayer.getRack();

    if (board.getTile(row, col) != nullptr) {
        std::cout << "Cell already occupied!" << std::endl;
        return false;
    }

    if (rack.empty()) {
        std::cout << "No tiles in rack to place!" << std::endl;
        return false;
    }

    Tile tileToPlace = rack[0]; // For simplicity, use the first tile in the rack

    if (board.placeTile(row, col, tileToPlace)) {
        currentPlayer.removeTileFromRack(0); // Remove the first tile
        currentWordPositions.push_back({row, col});

        std::cout << "Placed tile '" << tileToPlace.getLetter() 
                  << "' at (" << row << ", " << col << ")" << std::endl;
        std::cout << "Press ENTER to confirm word, or BACKSPACE to cancel" << std::endl;
        
        gameState = GameState::PLACING_TILES;
        return true;
    }

    return false;
}

bool Game::validateCurrentWord() {
    if (currentWordPositions.empty()) {
        std::cout << "No tiles placed yet!" << std::endl;
        return false;
    }

    std::string word = buildWordFromPositions();
    std::cout << "Validating word: " << word << std::endl;

    if (isValidWord(word)) {
        std::cout << "Word '" << word << "' is valid!" << std::endl;

        int wordScore = calculateWordScore(word, currentWordPositions[0].first, 
                                         currentWordPositions[0].second, "HORIZONTAL");

        getCurrentPlayer().addScore(wordScore);

        std::cout << "Score: " << wordScore << " points added!" << std::endl;
        std::cout << getCurrentPlayer().getName() 
                  << " total score: " << getCurrentPlayer().getScore() << std::endl;
        
        // Refill player's rack
        refillPlayerRack();

        // End turn
        currentWordPositions.clear();
        gameState = GameState::PLAYING;
        switchTurn();
        
        return true;
    } else {
        std::cout << word << "' is not in the dictionary!" << std::endl;
        std::cout << "Returning tiles to rack..." << std::endl;
        
        // Return tiles to rack
        cancelWord();
        return false;
    }
}

void Game::cancelWord() {
    Player& currentPlayer = getCurrentPlayer();
    
    // Return all placed tiles to rack
    for (const auto& pos : currentWordPositions) {
        const Tile* tile = board.getTile(pos.first, pos.second);
        if (tile) {
            currentPlayer.addTileToRack(*tile);
            board.removeTile(pos.first, pos.second); // You'll need to add this method
        }
    }
    
    currentWordPositions.clear();
    gameState = GameState::PLAYING;
    std::cout << "Word cancelled. Tiles returned to rack." << std::endl;
}

void Game::refillPlayerRack() {
    Player& currentPlayer = getCurrentPlayer();
    int tilesNeeded = 7 - static_cast<int>(currentPlayer.getRack().size());
    
    std::cout << "Drawing " << tilesNeeded << " new tiles..." << std::endl;
    
    for (int i = 0; i < tilesNeeded && !tileBag.empty(); i++) {
        Tile newTile = tileBag.front();
        tileBag.pop();
        currentPlayer.addTileToRack(newTile);
        std::cout << "  • Drew: " << newTile.getLetter() 
                  << "(" << newTile.getPoints() << ")" << std::endl;
    }
}

std::string Game::buildWordFromPositions() const {
    if (currentWordPositions.empty()) return "";
    
    std::string word;
    for (const auto& pos : currentWordPositions) {
        const Tile* tile = board.getTile(pos.first, pos.second);
        if (tile) {
            word += tile->getLetter();
        }
    }
    return word;
}

bool Game::handleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_RETURN: // ENTER key
            if (gameState == GameState::PLACING_TILES) {
                validateCurrentWord();
            }
            break;
            
        case SDLK_BACKSPACE:
            if (gameState == GameState::PLACING_TILES) {
                cancelWord();
            }
            break;
            
        case SDLK_ESCAPE:
            if (gameState == GameState::PLACING_TILES) {
                cancelWord();
            } else if (gameState == GameState::PLAYING) {
                gameState = GameState::PAUSED;
                std::cout << "Game paused. Press ESC again to quit." << std::endl;
            } else {
                isRunning = false;
            }
            break;
        case SDLK_S:
            getCurrentPlayer().shuffleRack();
            std::cout << "🔀 " << getCurrentPlayer().getName() << "'s rack manually shuffled!" << std::endl;
            break;
            
        // TESTING CONTROLS - Easy to understand
        case SDLK_H:
            printHelp();
            break;
            
        case SDLK_P:
            printGameState();
            break;
            
        case SDLK_1:
            placeTestWord();
            break;
            
        case SDLK_2:
            givePlayerTestTiles();
            break;
            
        case SDLK_3:
            testScoring();
            break;
            
        case SDLK_4:
            testDictionary();
            break;
            
        case SDLK_R:
            resetBoard();
            break;
            
        case SDLK_T:
            switchTurn();
            std::cout << "Switched to Player " << (getCurrentPlayerIndex() + 1) << std::endl;
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

const Player& Game::getPlayer1() const {
    return player1;
}

const Player& Game::getPlayer2() const {
    return player2;
}

int Game::getCurrentPlayerIndex() const {
    return currentPlayerIndex;
}

std::vector<TilePlacement> Game::getCurrentWord() const {
    std::vector<TilePlacement> placements; // Create local vector
    
    for (const auto& pos : currentWordPositions) {
        const Tile* tile = board.getTile(pos.first, pos.second);
        if (tile) {
            TilePlacement placement;
            placement.row = pos.first;
            placement.col = pos.second;
            placement.tile = tile;
            placements.push_back(placement);
        }
    }
    
    return placements; // Return by value
}