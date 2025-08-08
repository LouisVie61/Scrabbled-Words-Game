#include "Game.hpp"
#include "../UI/GameRenderer.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <set>

Game::Game() : gameState(GameState::MENU), gameMode(GameMode::HUMAN_VS_HUMAN),
               currentPlayerIndex(0), gameOver(false), consecutivePasses(0), consecutiveFailures(0),
               window(nullptr), renderer(nullptr), isRunning(false),
               selectedTileIndex(0), mouseX(0), mouseY(0), mouseOnBoard(false) {
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

    if (!loadDictionary("src/Constant/enable1.txt")) {
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
    
    if (gameState == GameState::MENU) {
        currentPlayerIndex = 0;
        gameOver = false;
        consecutivePasses = 0;
        consecutiveFailures = 0;
    }
    
    return true;
}

void Game::selectTileFromRack(int index) {
    const auto& rack = getCurrentPlayer().getRack();
    if (index >= 0 && index < static_cast<int>(rack.size())) {
        selectedTileIndex = index;
        std::cout << "Selected tile: " << rack[index].getLetter() 
                  << " at position " << (index + 1) << "/" << rack.size() << std::endl;
        
        // Show rack with new selection
        std::cout << "Rack: ";
        for (size_t i = 0; i < rack.size(); i++) {
            if (i == selectedTileIndex) {
                std::cout << "[>" << rack[i].getLetter() << "<] ";
            } else {
                std::cout << "[" << rack[i].getLetter() << "] ";
            }
        }
        std::cout << std::endl;
    } else {
        std::cout << "Invalid tile selection: " << (index + 1) << std::endl;
    }
}

int Game::getSelectedTileIndex() const {
    return selectedTileIndex;
}

void Game::selectNextTile() {
    const auto& rack = getCurrentPlayer().getRack();
    if (!rack.empty()) {
        selectedTileIndex = (selectedTileIndex + 1) % rack.size();
        std::cout << "Selected tile: " << rack[selectedTileIndex].getLetter() 
                  << " (position " << selectedTileIndex + 1 << "/" << rack.size() << ")" << std::endl;
    }
}

void Game::selectPreviousTile() {
    const auto& rack = getCurrentPlayer().getRack();
    if (!rack.empty()) {
        selectedTileIndex = (selectedTileIndex - 1 + rack.size()) % rack.size();
        std::cout << "Selected tile: " << rack[selectedTileIndex].getLetter() 
                  << " (position " << selectedTileIndex + 1 << "/" << rack.size() << ")" << std::endl;
    }
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
        SDL_Delay(16);
    }
}

bool Game::isGameRunning() const {
    return isRunning;
}

void Game::startNewGame() {
    // Reset board and game state
    std::cout<< "Starting a new game ..." << std::endl;

    board.clear();

    gameState = GameState::PLAYING;
    gameOver = false;
    consecutivePasses = 0;
    consecutiveFailures = 0;
    currentPlayerIndex = 0;
    selectedTileIndex = 0;

    currentWordPositions.clear();
    currentWord.clear();
    wordInProgress = false;

    setupGame(gameMode, player1.getName(), player2.getName());

    std::cout << "New game started! " << player1.getName() << " goes first." << std::endl;
    std::cout << "Board cleared, tiles redistributed!" << std::endl;
}

void Game::endGame() {
    gameState = GameState::GAME_OVER;
    gameOver = true;
    
    std::cout << "\nGAME OVER!" << std::endl;
    std::cout << "========== FINAL SCORE CALCULATION ==========" << std::endl;
    
    // Show scores before final adjustments
    std::cout << "Scores before final calculation:" << std::endl;
    std::cout << player1.getName() << ": " << player1.getScore() << " points" << std::endl;
    std::cout << player2.getName() << ": " << player2.getScore() << " points" << std::endl;
    
    // Calculate remaining tile penalties
    int player1TilePenalty = 0;
    int player2TilePenalty = 0;
    
    for (const auto& tile : player1.getRack()) {
        player1TilePenalty += tile.getPoints();
    }
    for (const auto& tile : player2.getRack()) {
        player2TilePenalty += tile.getPoints();
    }
    
    std::cout << "\nRemaining tile penalties:" << std::endl;
    std::cout << player1.getName() << ": -" << player1TilePenalty << " points" << std::endl;
    std::cout << player2.getName() << ": -" << player2TilePenalty << " points" << std::endl;
    
    player1.subtractScore(player1TilePenalty);
    player2.subtractScore(player2TilePenalty);
    
    // If someone used all their tiles, they get the opponent's tile points as bonus
    if (player1.getRackSize() == 0 && player2TilePenalty > 0) {
        player1.addScore(player2TilePenalty);
        std::cout << player1.getName() << " gets +" << player2TilePenalty 
                  << " bonus for using all tiles!" << std::endl;
    } else if (player2.getRackSize() == 0 && player1TilePenalty > 0) {
        player2.addScore(player1TilePenalty);
        std::cout << player2.getName() << " gets +" << player1TilePenalty 
                  << " bonus for using all tiles!" << std::endl;
    }
    
    // Final scores
    std::cout << "\n========== FINAL SCORES ==========" << std::endl;
    std::cout << player1.getName() << ": " << player1.getScore() << " points" << std::endl;
    std::cout << player2.getName() << ": " << player2.getScore() << " points" << std::endl;
    
    determineWinner();
}

void Game::determineWinner() {
    int score1 = player1.getScore();
    int score2 = player2.getScore();
    
    if (score1 > score2) {
        std::cout << "\n" << player1.getName() << " WINS!" << std::endl;
        std::cout << "Victory margin: " << (score1 - score2) << " points" << std::endl;
    } else if (score2 > score1) {
        std::cout << "\n" << player2.getName() << " WINS!" << std::endl;
        std::cout << "Victory margin: " << (score2 - score1) << " points" << std::endl;
    } else {
        std::cout << "\nSCORES ARE TIED!" << std::endl;
        std::cout << "Applying tiebreaker rules..." << std::endl;
        
        int tiles1 = player1.getRackSize();
        int tiles2 = player2.getRackSize();
        
        if (tiles1 < tiles2) {
            std::cout << player1.getName() << " wins the tiebreaker!" << std::endl;
            std::cout << "Reason: Fewer remaining tiles (" << tiles1 << " vs " << tiles2 << ")" << std::endl;
            player1.addScore(1);
        } else if (tiles2 < tiles1) {
            std::cout << player2.getName() << " wins the tiebreaker!" << std::endl;
            std::cout << "Reason: Fewer remaining tiles (" << tiles2 << " vs " << tiles1 << ")" << std::endl;
            player2.addScore(1);
        } else {
            int value1 = 0, value2 = 0;
            for (const auto& tile : player1.getRack()) {
                value1 += tile.getPoints();
            }
            for (const auto& tile : player2.getRack()) {
                value2 += tile.getPoints();
            }
            
            if (value1 < value2) {
                std::cout << player1.getName() << " wins the tiebreaker!" << std::endl;
                std::cout << "Reason: Lower remaining tile value (" << value1 << " vs " << value2 << ")" << std::endl;
                player1.addScore(1);
            } else if (value2 < value1) {
                std::cout << player2.getName() << " wins the tiebreaker!" << std::endl;
                std::cout << "Reason: Lower remaining tile value (" << value2 << " vs " << value1 << ")" << std::endl;
                player2.addScore(1);
            } else {
                std::cout << "TRUE TIE! Both players performed equally well!" << std::endl;
                std::cout << "Both players are declared winners!" << std::endl;
            }
        }
    }
    
    std::cout << "============================================" << std::endl;
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
    std::cout << getCurrentPlayer().getName() << "'s tiles shuffled!" << std::endl;
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
    if (!isValidWord(word)) {
        return false;
    }
    
    std::vector<Tile> tiles;
    for (int index : tileIndices) {
        Tile* tile = getCurrentPlayer().getTileFromRack(index);
        if (!tile) return false;
        tiles.push_back(*tile);
    }
    
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
    
    auto sortedIndices = tileIndices;
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());
    for (int index : sortedIndices) {
        getCurrentPlayer().removeTileFromRack(index);
    }
    
    drawTilesForPlayer(getCurrentPlayer(), static_cast<int>(tileIndices.size()));
    
    consecutivePasses = 0;
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
    
    consecutivePasses = 0;
    
    return true;
}

void Game::skipTurn() {
    std::string currentPlayerName = getCurrentPlayer().getName();
    
    std::cout << currentPlayerName << " skipped their turn." << std::endl;
    
    consecutivePasses++;
    std::cout << "Consecutive passes: " << consecutivePasses << "/" << MAX_CONSECUTIVE_PASSES << std::endl;
    
    // Check for game end before switching turns
    if (consecutivePasses >= MAX_CONSECUTIVE_PASSES) {
        std::cout << "🔚 Game ending due to " << MAX_CONSECUTIVE_PASSES << " consecutive passes!" << std::endl;
        endGame();
        return;
    }
    
    refreshBothPlayerRacks();
    switchTurn();
    
    std::cout << "Now it's " << getCurrentPlayer().getName() << "'s turn." << std::endl;
    
    // Show the new player's rack
    const auto& rack = getCurrentPlayer().getRack();
    std::cout << "Your tiles: ";
    for (size_t i = 0; i < rack.size(); i++) {
        if (i == selectedTileIndex) {
            std::cout << "[>" << rack[i].getLetter() << "<](" << rack[i].getPoints() << ") ";
        } else {
            std::cout << "[" << rack[i].getLetter() << "](" << rack[i].getPoints() << ") ";
        }
    }
    std::cout << std::endl;
    
    // Reset selected tile index if it's out of bounds for new player
    if (selectedTileIndex >= static_cast<int>(rack.size()) && !rack.empty()) {
        selectedTileIndex = 0;
        std::cout << "Selected: " << rack[selectedTileIndex].getLetter() 
                  << " at position 1/" << rack.size() << std::endl;
    }
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
            gameRenderer->renderGameStart();
            break;
        case GameState::PLAYING:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderSelectedTileIndicator(*this);
            gameRenderer->renderPlayerInfo(player1, player2, currentPlayerIndex);
            gameRenderer->renderCurrentWordScore(*this);
            gameRenderer->renderPauseButton();
            break;
        case GameState::PLACING_TILES:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPickedTiles(*this);
            gameRenderer->renderTilePreview(*this, mouseX, mouseY);
            gameRenderer->renderSelectedTileIndicator(*this);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderPlayerInfo(player1, player2, currentPlayerIndex);
            gameRenderer->renderCurrentWordScore(*this);
            gameRenderer->renderPauseButton();
            break;
        case GameState::VALIDATING_WORD:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderGameState(*this);
            break;
        case GameState::GAME_OVER:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerInfo(player1, player2, currentPlayerIndex);
            gameRenderer->renderGameOver(player1, player2);
            break;
        case GameState::PAUSED:
            gameRenderer->renderBoard(board);
            gameRenderer->renderPlayerRacks(player1, player2, currentPlayerIndex);
            gameRenderer->renderPlayerInfo(player1, player2, currentPlayerIndex);
            gameRenderer->renderPauseMenu();
            break;
    }
    
    gameRenderer->present();
}

// Add these methods to Game.cpp:

void Game::printHelp() const {
    std::cout << "\n========== SCRABBLE GAME CONTROLS ==========" << std::endl;
    std::cout << "TILE SELECTION:" << std::endl;
    std::cout << "  1-7 - Select specific tile from rack" << std::endl;
    std::cout << "  LEFT/RIGHT ARROWS - Navigate through tiles" << std::endl;
    std::cout << "\nGAME PLAY:" << std::endl;
    std::cout << "  MOUSE CLICK - Place selected tile on board" << std::endl;
    std::cout << "  ENTER - Confirm word placement" << std::endl;
    std::cout << "  BACKSPACE - Cancel current word" << std::endl;
    std::cout << "  S - Shuffle current player's rack" << std::endl;
    std::cout << "  ESC - Pause/Quit game" << std::endl;
    std::cout << "\n======== TESTING CONTROLS (when not playing) ========" << std::endl;
    std::cout << "  H - Show this help" << std::endl;
    std::cout << "  P - Print current game state" << std::endl;
    std::cout << "  R - Reset/clear the board" << std::endl;
    std::cout << "  T - Switch turns between players" << std::endl;
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
    std::cout << "Consecutive failures: " << consecutiveFailures << std::endl; // Add this line
    
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
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Store mouse position for rendering
                    mouseX = static_cast<int>(event.button.x);
                    mouseY = static_cast<int>(event.button.y);
                    
                    std::cout << "Mouse click detected in state: " << static_cast<int>(gameState) 
                              << " at (" << mouseX << ", " << mouseY << ")" << std::endl;
                    
                    handleMouseClick(mouseX, mouseY);
                }
                break;
                
            case SDL_EVENT_MOUSE_MOTION:
                mouseX = static_cast<int>(event.motion.x);
                mouseY = static_cast<int>(event.motion.y);
                break;
                
            case SDL_EVENT_KEY_DOWN:
                handleKeyPress(event.key.key);
                break;
        }
    }
}

bool Game::handleMouseClick(int x, int y) {
    std::cout << "handleMouseClick called: (" << x << ", " << y << ") in state " << static_cast<int>(gameState) << std::endl;

    // === MENU STATE HANDLING ===
    if (gameState == GameState::MENU) {
        std::cout << "In MENU state, checking buttons..." << std::endl;
        
        if (gameRenderer->isPointInStartButton(x, y)) {
            std::cout << "START PLAYING clicked!" << std::endl;
            if (!setupGame(GameMode::HUMAN_VS_HUMAN, "Player 1", "Player 2")) {
                std::cerr << "Failed to setup game!" << std::endl;
                return false;
            }
            gameState = GameState::PLAYING;
            return true;
        }
        
        if (gameRenderer->isPointInTutorialButton(x, y)) {
            std::cout << "HOW TO PLAY clicked - toggling tutorial!" << std::endl;
            GameRenderer::toggleTutorial();
            return true;
        }
        
        if (gameRenderer->isPointInExitButton(x, y)) {
            std::cout << "EXIT GAME clicked!" << std::endl;
            isRunning = false;
            return true;
        }
        
        std::cout << "Clicked elsewhere on menu" << std::endl;
        return false; // Don't start game on random clicks
    }
    
    // === PAUSE STATE HANDLING ===
    if (gameState == GameState::PAUSED) {
        if (handlePauseMenuClick(x, y)) {
            return true;
        }
        // If click was outside pause menu, resume game
        gameState = GameState::PLAYING;
        std::cout << "Game resumed by clicking outside pause menu!" << std::endl;
        return true;
    }
    
    // === GAME OVER STATE HANDLING ===
    if (gameState == GameState::GAME_OVER) {
        std::cout << "In GAME_OVER state, checking buttons..." << std::endl;
        
        if (gameRenderer->isPointInPlayAgainButton(x, y)) {
            std::cout << "PLAY AGAIN clicked!" << std::endl;
            startNewGame();
            return true;
        }
        
        if (gameRenderer->isPointInMainMenuButton(x, y)) {
            std::cout << "MAIN MENU clicked!" << std::endl;
            gameState = GameState::MENU;
            return true;
        }
        
        if (gameRenderer->isPointInGameOverExitButton(x, y)) {
            std::cout << "EXIT GAME clicked!" << std::endl;
            isRunning = false;
            return true;
        }
        
        std::cout << "Clicked elsewhere on game over screen" << std::endl;
        return false;
    }
    
    // === PAUSE BUTTON CHECK (for playing states) ===
    if (gameRenderer && gameRenderer->isPointInPauseButton(x, y)) {
        if (gameState == GameState::PLAYING || gameState == GameState::PLACING_TILES) {
            gameState = GameState::PAUSED;
            std::cout << "Game paused via button click" << std::endl;
            return true;
        }
    }
    
    // === PLAYING STATE HANDLING ===
    if (gameState != GameState::PLAYING && gameState != GameState::PLACING_TILES) {
        std::cout << "Not in a playable state" << std::endl;
        return false;
    }
    
    // Check if click is on board
    int row, col;
    if (gameRenderer->isPointInBoard(x, y, row, col)) {
        std::cout << "Clicked on board cell: (" << row << ", " << col << ")" << std::endl;
        
        if (gameState == GameState::PLAYING) {
            startWordPlacement();
        }
        
        return placeTileFromRack(row, col);
    }
    
    // Check if click is on rack
    int rackIndex = getRackTileIndexFromMouse(x, y);
    if (rackIndex >= 0) {
        std::cout << "Clicked on rack tile: " << rackIndex << std::endl;
        selectTileFromRack(rackIndex);
        return true;
    }
    
    std::cout << "Clicked outside interactive areas" << std::endl;
    return false;
}

bool Game::handlePauseMenuClick(int x, int y) {
    PauseMenuOption option = gameRenderer->getPauseMenuOption(x, y);
    
    switch (option) {
        case PauseMenuOption::CONTINUE:
            gameState = GameState::PLAYING;
            std::cout << "Game resumed" << std::endl;
            return true;
            
        case PauseMenuOption::SURRENDER:
            // Current player surrenders
            std::cout << getCurrentPlayer().getName() << " surrendered!" << std::endl;
            getOtherPlayer().addScore(100); // Bonus for opponent
            endGame();
            return true;
            
        case PauseMenuOption::NEW_GAME:
            std::cout << "Starting new game..." << std::endl;
            startNewGame();
            return true;
            
        case PauseMenuOption::QUIT:
            std::cout << "Quitting game..." << std::endl;
            isRunning = false;
            return true;
            
        default:
            return false;
    }
}

int Game::getRackTileIndexFromMouse(int mouseX, int mouseY) const {
    const float TILE_SPACING = 40.0f;
    const float RACK_PADDING = 50.0f;
    const float TILE_SIZE = 35.0f;
    
    // Calculate rack position to match GameRenderer::renderPlayerRacks
    const float BOARD_SIZE = 15;
    const float CELL_SIZE = 35;
    const float BOARD_OFFSET_X = 150.0f;  // Updated to match GameRenderer
    const float BOARD_OFFSET_Y = 80.0f;   // Updated to match GameRenderer
    
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    
    const float rackY = BOARD_OFFSET_Y + boardHeight + 50.0f;
    const float totalRackWidth = 7.0f * TILE_SPACING;
    
    // Center the rack but ensure it fits within screen bounds
    float rackStartX = (WINDOW_WIDTH - totalRackWidth) / 2.0f;
    
    // Ensure minimum margins (same logic as renderPlayerRacks)
    if (rackStartX < 30.0f) {
        rackStartX = 30.0f;
    } else if (rackStartX + totalRackWidth > WINDOW_WIDTH - 30.0f) {
        rackStartX = WINDOW_WIDTH - totalRackWidth - 30.0f;
    }
    
    // Check if mouse is in the rack area vertically
    if (mouseY < rackY || mouseY > rackY + TILE_SIZE) {
        return -1; // Not in rack area
    }
    
    const auto& rack = getCurrentPlayer().getRack();
    const float actualRackWidth = static_cast<float>(rack.size()) * TILE_SPACING;
    const float centerOffset = (totalRackWidth - actualRackWidth) / 2.0f;
    
    // Calculate which tile was clicked    
    for (size_t i = 0; i < rack.size(); i++) {
        float tileX = rackStartX + centerOffset + (i * TILE_SPACING);
        
        if (mouseX >= tileX && mouseX <= tileX + TILE_SIZE) {
            return static_cast<int>(i);
        }
    }
    
    return -1; // No tile clicked
}

void Game::startWordPlacement() {
    if (gameState == GameState::PLAYING) {
        currentWordPositions.clear();
        currentWord.clear();
        wordInProgress = true;
        gameState = GameState::PLACING_TILES;
        
        std::cout << "Starting word placement..." << std::endl;
        std::cout << "Use 1-7 keys or LEFT/RIGHT arrows to select tiles" << std::endl;
        std::cout << "Click on board cells to place selected tile" << std::endl;
        std::cout << "Press ENTER to confirm word, BACKSPACE to cancel" << std::endl;
        
        // Show current player's rack with selection indicator
        const auto& rack = getCurrentPlayer().getRack();
        std::cout << "Your tiles: ";
        for (size_t i = 0; i < rack.size(); i++) {
            if (i == selectedTileIndex) {
                std::cout << "[>" << rack[i].getLetter() << "<](" << rack[i].getPoints() << ") ";
            } else {
                std::cout << "[" << rack[i].getLetter() << "](" << rack[i].getPoints() << ") ";
            }
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

    if (selectedTileIndex >= static_cast<int>(rack.size())) {
        selectedTileIndex = 0;
    }

    // Show which tile we're about to place
    std::cout << "Placing tile: " << rack[selectedTileIndex].getLetter() 
              << " from position " << selectedTileIndex + 1 << std::endl;


    Tile tileToPlace = rack[selectedTileIndex];

    if (board.placeTile(row, col, tileToPlace)) {
        currentPlayer.removeTileFromRack(selectedTileIndex);
        currentWordPositions.push_back({row, col});

        std::cout << "Placed tile '" << tileToPlace.getLetter() 
                  << "' at (" << row << ", " << col << ")" << std::endl;
        std::cout << "Press ENTER to confirm word, or BACKSPACE to cancel" << std::endl;
        
        // FIX: Update selectedTileIndex after tile removal
        if (!rack.empty()) {
            if (selectedTileIndex >= static_cast<int>(rack.size())) {
                selectedTileIndex = rack.size() - 1;
            }
            
            std::cout << "Updated rack: ";
            for (size_t i = 0; i < rack.size(); i++) {
                if (i == selectedTileIndex) {
                    std::cout << "[>" << rack[i].getLetter() << "<] ";
                } else {
                    std::cout << "[" << rack[i].getLetter() << "] ";
                }
            }
            std::cout << std::endl;
            
            std::cout << "Now selected: " << rack[selectedTileIndex].getLetter() 
                      << " at position " << (selectedTileIndex + 1) << "/" << rack.size() << std::endl;
        } else {
            selectedTileIndex = 0;
            std::cout << "Rack is now empty!" << std::endl;
        }
        
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

    // Find ALL words formed by this move (main word + cross-words)
    std::vector<WordInfo> allWords = findAllWordsFormed();
    
    if (allWords.empty()) {
        std::cout << "No valid words formed!" << std::endl;
        return false;
    }

    // Validate ALL words must be in dictionary
    std::cout << "Checking all words formed:" << std::endl;
    for (const auto& wordInfo : allWords) {
        std::cout << "  '" << wordInfo.word << "' (" 
                  << (wordInfo.isHorizontal ? "horizontal" : "vertical") << ")" << std::endl;
        
        if (!isValidWord(wordInfo.word)) {
            std::cout << "Invalid word found: '" << wordInfo.word << "'" << std::endl;
            std::cout << "All words must be valid! Canceling move..." << std::endl;
            cancelWord();
            consecutiveFailures++;
            getCurrentPlayer().shuffleRack();
            
            if (checkFailureGameEnd()) {
                return false;
            }
            return false;
        }
    }

    // Calculate total score from ALL words
    int totalScore = 0;
    std::cout << "All words are valid! Calculating scores:" << std::endl;
    
    for (const auto& wordInfo : allWords) {
        int wordScore = calculateWordScore(wordInfo);
        totalScore += wordScore;
        std::cout << "  '" << wordInfo.word << "' = " << wordScore << " points" << std::endl;
    }

    getCurrentPlayer().addScore(totalScore);
    
    std::cout << "Total score: " << totalScore << " points added!" << std::endl;
    std::cout << getCurrentPlayer().getName() 
              << " total score: " << getCurrentPlayer().getScore() << std::endl;

    currentWordPositions.clear();
    gameState = GameState::PLAYING;
    
    handleTurnCompletion(true);
    return true;
}

std::vector<WordInfo> Game::findAllWordsFormed() const {
    std::vector<WordInfo> allWords;
    std::set<std::string> processedWords;
    
    if (currentWordPositions.empty()) return allWords;
    
    auto sortedPositions = currentWordPositions;
    std::sort(sortedPositions.begin(), sortedPositions.end(), 
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            if (a.first == b.first) return a.second < b.second;
            return a.first < b.first;
        });
    
    bool mainIsHorizontal = (sortedPositions[0].first == sortedPositions.back().first);
    
    WordInfo mainWord = findWordAtPosition(
        sortedPositions[0].first, 
        sortedPositions[0].second, 
        mainIsHorizontal
    );
    
    if (mainWord.word.length() > 1) {
        for (const auto& pos : currentWordPositions) {
            if (std::find(mainWord.positions.begin(), mainWord.positions.end(), pos) != mainWord.positions.end()) {
                mainWord.newTilePositions.push_back(pos);
            }
        }
        allWords.push_back(mainWord);
        processedWords.insert(mainWord.word + std::to_string(mainWord.startRow) + std::to_string(mainWord.startCol));
    }

    for (const auto& pos : currentWordPositions) {
        WordInfo crossWord = findWordAtPosition(pos.first, pos.second, !mainIsHorizontal);
        
        if (crossWord.word.length() > 1) {
            std::string key = crossWord.word + std::to_string(crossWord.startRow) + std::to_string(crossWord.startCol);
            if (processedWords.find(key) == processedWords.end()) {
                crossWord.newTilePositions.push_back(pos);
                allWords.push_back(crossWord);
                processedWords.insert(key);
            }
        }
    }
    
    return allWords;
}

WordInfo Game::findWordAtPosition(int row, int col, bool horizontal) const {
    WordInfo wordInfo;
    wordInfo.isHorizontal = horizontal;
    
    if (horizontal) {
        int startCol = col;
        int endCol = col;
        
        while (startCol > 0 && board.getTile(row, startCol - 1) != nullptr) {
            startCol--;
        }
        
        while (endCol < 14 && board.getTile(row, endCol + 1) != nullptr) {
            endCol++;
        }
        
        wordInfo.startRow = row;
        wordInfo.startCol = startCol;
        
        for (int c = startCol; c <= endCol; c++) {
            const Tile* tile = board.getTile(row, c);
            if (tile) {
                wordInfo.word += tile->getLetter();
                wordInfo.positions.push_back({row, c});
            }
        }
    } else {
        int startRow = row;
        int endRow = row;
        
        while (startRow > 0 && board.getTile(startRow - 1, col) != nullptr) {
            startRow--;
        }
        
        while (endRow < 14 && board.getTile(endRow + 1, col) != nullptr) {
            endRow++;
        }
        
        wordInfo.startRow = startRow;
        wordInfo.startCol = col;
        
        for (int r = startRow; r <= endRow; r++) {
            const Tile* tile = board.getTile(r, col);
            if (tile) {
                wordInfo.word += tile->getLetter();
                wordInfo.positions.push_back({r, col});
            }
        }
    }
    
    return wordInfo;
}

int Game::calculateWordScore(const WordInfo& wordInfo) const {
    int score = 0;
    int wordMultiplier = 1;
    
    for (const auto& pos : wordInfo.positions) {
        const Tile* tile = board.getTile(pos.first, pos.second);
        if (tile) {
            int letterPoints = tile->getPoints();
            
            bool isNewlyPlaced = std::find(wordInfo.newTilePositions.begin(), 
                                         wordInfo.newTilePositions.end(), 
                                         pos) != wordInfo.newTilePositions.end();
            
            if (isNewlyPlaced) {
                SpecialSquare special = board.getSpecialSquare(pos.first, pos.second);
                
                switch (special) {
                    case SpecialSquare::DOUBLE_LETTER:
                        letterPoints *= 2;
                        std::cout << "    " << tile->getLetter() << " gets double letter bonus" << std::endl;
                        break;
                    case SpecialSquare::TRIPLE_LETTER:
                        letterPoints *= 3;
                        std::cout << "    " << tile->getLetter() << " gets triple letter bonus" << std::endl;
                        break;
                    case SpecialSquare::DOUBLE_WORD:
                    case SpecialSquare::CENTER:
                        wordMultiplier *= 2;
                        std::cout << "    Word '" << wordInfo.word << "' gets double word bonus" << std::endl;
                        break;
                    case SpecialSquare::TRIPLE_WORD:
                        wordMultiplier *= 3;
                        std::cout << "    Word '" << wordInfo.word << "' gets triple word bonus" << std::endl;
                        break;
                    default:
                        break;
                }
            }
            
            score += letterPoints;
        }
    }
    
    return score * wordMultiplier;
}

void Game::cancelWord() {
    Player& currentPlayer = getCurrentPlayer();
    
    // Return all placed tiles to rack
    for (const auto& pos : currentWordPositions) {
        const Tile* tile = board.getTile(pos.first, pos.second);
        if (tile) {
            currentPlayer.addTileToRack(*tile);
            board.removeTile(pos.first, pos.second);
        }
    }
    
    currentWordPositions.clear();
    gameState = GameState::PLAYING;
    std::cout << "Word cancelled. Tiles returned to rack." << std::endl;
}

std::string Game::buildWordFromPositions() const {
    if (currentWordPositions.empty()) return "";
    
    auto sortedPositions = currentWordPositions;
    std::sort(sortedPositions.begin(), sortedPositions.end(), 
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            if (a.first == b.first) return a.second < b.second; 
            return a.first < b.first;
        });
    
    bool isHorizontal = (sortedPositions[0].first == sortedPositions.back().first);
    
    if (isHorizontal) {
        int row = sortedPositions[0].first;
        int startCol = sortedPositions[0].second;
        int endCol = sortedPositions.back().second;
        
        while (startCol > 0 && board.getTile(row, startCol - 1) != nullptr) {
            startCol--;
        }
        
        while (endCol < 14 && board.getTile(row, endCol + 1) != nullptr) {
            endCol++;
        }
        
        std::string word;
        for (int col = startCol; col <= endCol; col++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                word += tile->getLetter();
            } else {
                std::cerr << "Error: Missing tile at (" << row << ", " << col << ")" << std::endl;
                return word;
            }
        }
        return word;
        
    } else {
        int col = sortedPositions[0].second;
        int startRow = sortedPositions[0].first;
        int endRow = sortedPositions.back().first;
        
        while (startRow > 0 && board.getTile(startRow - 1, col) != nullptr) {
            startRow--;
        }
        
        while (endRow < 14 && board.getTile(endRow + 1, col) != nullptr) {
            endRow++;
        }
        
        std::string word;
        for (int row = startRow; row <= endRow; row++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                word += tile->getLetter();
            } else {
                std::cerr << "Error: Missing tile at (" << row << ", " << col << ")" << std::endl;
                return word;
            }
        }
        return word;
    }
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
            if (gameState == GameState::PAUSED) {
                std::cout << "Quitting game..." << std::endl;
                isRunning = false;
            } else if (gameState == GameState::PLACING_TILES) {
                cancelWord();
            } else if (gameState == GameState::PLAYING) {
                gameState = GameState::PAUSED;
                std::cout << "Game paused. Click menu options or press ESC again to quit." << std::endl;
            } else {
                isRunning = false;
            }
            break;
        case SDLK_SPACE:
            if (gameState == GameState::PLAYING) {
                skipTurn();
            } else if (gameState == GameState::PAUSED) {
                gameState = GameState::PLAYING;
                std::cout << "Game resumed!" << std::endl;
            }
            break;
        case SDLK_S:
            getCurrentPlayer().shuffleRack();
            {
                const auto& rack = getCurrentPlayer().getRack();
                if (selectedTileIndex >= static_cast<int>(rack.size())) {
                    selectedTileIndex = rack.empty() ? 0 : rack.size() - 1;
                }
                
                std::cout << getCurrentPlayer().getName() << "'s rack shuffled!" << std::endl;
                
                if (!rack.empty()) {
                    std::cout << "Selected tile is now: " << rack[selectedTileIndex].getLetter() 
                              << " at position " << (selectedTileIndex + 1) << "/" << rack.size() << std::endl;
                    
                    std::cout << "Rack: ";
                    for (size_t i = 0; i < rack.size(); i++) {
                        if (i == selectedTileIndex) {
                            std::cout << "[>" << rack[i].getLetter() << "<] ";
                        } else {
                            std::cout << "[" << rack[i].getLetter() << "] ";
                        }
                    }
                    std::cout << std::endl;
                }
            }
            break;
        case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
        case SDLK_5: case SDLK_6: case SDLK_7:
            if (gameState == GameState::PLAYING || gameState == GameState::PLACING_TILES) {
                int tileIndex = key - SDLK_1;
                selectTileFromRack(tileIndex);
            } else {
                if (key == SDLK_1) placeTestWord();
                else if (key == SDLK_2) givePlayerTestTiles();
                else if (key == SDLK_3) testScoring();
                else if (key == SDLK_4) testDictionary();
            }
            break;
        case SDLK_H:
            printHelp();
            break;
            
        case SDLK_P:
            printGameState();
            break;
            
        case SDLK_R:
            resetBoard();
            break;
            
        case SDLK_T:
            switchTurn();
            std::cout << "Switched to Player " << (getCurrentPlayerIndex() + 1) << std::endl;
            break;

        case SDLK_LEFT:
            if (gameState == GameState::PLAYING || gameState == GameState::PLACING_TILES) {
                selectPreviousTile();
            }
            break;
            
        case SDLK_RIGHT:
            if (gameState == GameState::PLAYING || gameState == GameState::PLACING_TILES) {
                selectNextTile();
            }
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

size_t Game::getTileBagSize() const {
    return tileBag.size();
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

void Game::refreshBothPlayerRacks() {
    std::cout << "Refreshing both players' racks..." << std::endl;
    player1.shuffleRack();
    player2.shuffleRack();
    
    while (player1.getRackSize() < 7 && !tileBag.empty()) {
        drawTilesForPlayer(player1, 1);
    }
    while (player2.getRackSize() < 7 && !tileBag.empty()) {
        drawTilesForPlayer(player2, 1);
    }
    
    std::cout << "Both players' racks have been refreshed and filled!" << std::endl;
}

void Game::handleTurnCompletion(bool wordSuccess) {
    if (wordSuccess) {
        consecutivePasses = 0;
        consecutiveFailures = 0;
        
        refreshBothPlayerRacks();
        if (checkGameEnd()) {
            return;
        }
        switchTurn();
    }
}

bool Game::checkFailureGameEnd() {
    if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
        std::cout << "🔚 Game ending due to " << MAX_CONSECUTIVE_FAILURES 
                  << " consecutive word validation failures!" << std::endl;
        
        Player& winner = getOtherPlayer();
        winner.addScore(50);
        
        std::cout << winner.getName() << " wins due to opponent's failures!" << std::endl;
        endGame();
        return true;
    }
    return false;
}