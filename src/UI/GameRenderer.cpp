#include "GameRenderer.hpp"
#include <iostream>
#include <vector>

// === STATIC COLOR DEFINITIONS ===
const SDL_Color GameRenderer::BOARD_COLOR = {139, 69, 19, 255};      // Brown
const SDL_Color GameRenderer::TILE_COLOR = {255, 248, 220, 255};     // Cornsilk
const SDL_Color GameRenderer::TEXT_COLOR = {0, 0, 0, 255};           // Black
const SDL_Color GameRenderer::WHITE_COLOR = {255, 255, 255, 255};    // White
const SDL_Color GameRenderer::BLACK_COLOR = {0, 0, 0, 255};          // Black
const SDL_Color GameRenderer::YELLOW_COLOR = {255, 255, 0, 150};     // Yellow (transparent)

const SDL_Color GameRenderer::SPECIAL_SQUARE_COLORS[] = {
    {255, 255, 255, 255},    // NORMAL - White
    {173, 216, 230, 255},    // DOUBLE_LETTER - Light Blue  
    {30, 144, 255, 255},     // TRIPLE_LETTER - Dodger Blue (darker)
    {255, 182, 193, 255},    // DOUBLE_WORD - Light Pink
    {220, 20, 60, 255},      // TRIPLE_WORD - Crimson (darker red)
    {255, 215, 0, 255}       // CENTER - Gold
};

// Define font sizes (missing from header but used in implementation)
static const int NORMAL_FONT_SIZE = 16;
static const int SMALL_FONT_SIZE = 12;

// === CONSTRUCTOR & DESTRUCTOR ===
GameRenderer::GameRenderer(SDL_Renderer* renderer, SDL_Window* window) 
    : renderer(renderer), window(window), font(nullptr), smallFont(nullptr) {
    initializeFonts();
}

GameRenderer::~GameRenderer() {
    cleanupFonts();
}

// === INITIALIZATION METHODS ===
bool GameRenderer::initializeFonts() {
    if (!initializeTTF()) {
        return false;
    }
    return loadFontsFromPaths();
}

void GameRenderer::cleanupFonts() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (smallFont) {
        TTF_CloseFont(smallFont);
    }
    TTF_Quit();
}

// === MAIN RENDERING METHODS ===
void GameRenderer::renderGameState(const Game& game) {
    clear();    
    renderBoard(game.getBoard());
    renderPickedTiles(game);
    renderPlayerRacks(game.getPlayer1(), game.getPlayer2(), game.getCurrentPlayerIndex());
    renderScores(game.getPlayer1(), game.getPlayer2());
    present();
}

void GameRenderer::renderBoard(const Board& board) {
    renderGrid();
    renderSpecialSquares(board);
    renderTiles(board);
}

void GameRenderer::renderPickedTiles(const Game& game) {
    // Get the current word being formed
    const auto& currentWord = game.getCurrentWord();
    
    // Render each picked tile with highlighting
    for (const auto& placement : currentWord) {
        int row = placement.row;
        int col = placement.col;
        const Tile* tile = placement.tile;
        
        if (tile) {
            // Get the cell position
            const SDL_FRect cellRect = getBoardCellRect(row, col);
            
            // Draw yellow highlight background for picked tiles
            SDL_SetRenderDrawColor(renderer, YELLOW_COLOR.r, YELLOW_COLOR.g, YELLOW_COLOR.b, YELLOW_COLOR.a);
            const SDL_FRect highlightRect = {
                cellRect.x - 2.0f, cellRect.y - 2.0f, 
                cellRect.w + 4.0f, cellRect.h + 4.0f
            };
            SDL_RenderFillRect(renderer, &highlightRect);
            
            // Render the tile on top of the highlight
            renderTile(cellRect.x, cellRect.y, tile);
        }
    }
}

void GameRenderer::renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer) {
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    const float bottomY = BOARD_OFFSET_Y + boardHeight + RACK_PADDING;
    const float topY = BOARD_OFFSET_Y - CELL_SIZE - RACK_PADDING;

    renderPlayerRack(player1, BOARD_OFFSET_X, bottomY, currentPlayer == 0);
    renderPlayerRack(player2, BOARD_OFFSET_X, topY, currentPlayer == 1);
}

void GameRenderer::renderScores(const Player& player1, const Player& player2) {
    const float rackWidth = 7 * TILE_SPACING;
    const float scoreX = BOARD_OFFSET_X + rackWidth + RACK_PADDING;
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    
    // Player 1 score (bottom)
    const float bottomY = BOARD_OFFSET_Y + boardHeight + RACK_PADDING;
    const SDL_FRect scoreRect1 = {scoreX, bottomY, SCORE_WIDTH, SCORE_HEIGHT};
    renderPlayerScore(player1, scoreRect1);
    
    // Player 2 score (top)
    const float topY = BOARD_OFFSET_Y - SCORE_HEIGHT - RACK_PADDING;
    const SDL_FRect scoreRect2 = {scoreX, topY, SCORE_WIDTH, SCORE_HEIGHT};
    renderPlayerScore(player2, scoreRect2);
}

// === SCREEN RENDERING METHODS ===
void GameRenderer::renderMenu() {
    renderMenuBackground();
    renderMenuContent();
}

void GameRenderer::renderGameOver(const Player& player1, const Player& player2) {
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    const SDL_FRect overlay = {0.0f, 0.0f, 1024.0f, 768.0f};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Game over box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_FRect gameOverRect = {250.0f, 200.0f, 500.0f, 300.0f};
    SDL_RenderFillRect(renderer, &gameOverRect);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &gameOverRect);
    
    // Content
    renderText("GAME OVER", 420.0f, 230.0f, BLACK_COLOR, font);
    
    std::string winnerText;
    if (player1.getScore() > player2.getScore()) {
        winnerText = player1.getName() + " WINS!";
    } else if (player2.getScore() > player1.getScore()) {
        winnerText = player2.getName() + " WINS!";
    } else {
        winnerText = "IT'S A TIE!";
    }
    renderText(winnerText, 380.0f, 280.0f, BLACK_COLOR, font);
    
    // Final scores
    renderText("Final Scores:", 360.0f, 320.0f, BLACK_COLOR, smallFont);
    const std::string score1 = player1.getName() + ": " + std::to_string(player1.getScore());
    const std::string score2 = player2.getName() + ": " + std::to_string(player2.getScore());
    renderText(score1, 300.0f, 350.0f, BLACK_COLOR, smallFont);
    renderText(score2, 300.0f, 370.0f, BLACK_COLOR, smallFont);
    
    renderText("Press ESC to exit", 380.0f, 420.0f, BLACK_COLOR, smallFont);
}

void GameRenderer::renderPauseScreen() {
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    const SDL_FRect overlay = {0.0f, 0.0f, 1024.0f, 768.0f};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Pause box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_FRect pauseRect = {350.0f, 300.0f, 300.0f, 150.0f};
    SDL_RenderFillRect(renderer, &pauseRect);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &pauseRect);
    
    renderText("GAME PAUSED", 420.0f, 330.0f, BLACK_COLOR, font);
    renderText("Press SPACE to resume", 380.0f, 360.0f, BLACK_COLOR, smallFont);
    renderText("Press ESC to quit", 400.0f, 380.0f, BLACK_COLOR, smallFont);
}

// === UTILITY METHODS ===
void GameRenderer::clear() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void GameRenderer::present() {
    SDL_RenderPresent(renderer);
}

SDL_FRect GameRenderer::getBoardCellRect(int row, int col) const {
    return {
        static_cast<float>(BOARD_OFFSET_X + col * CELL_SIZE + 1),
        static_cast<float>(BOARD_OFFSET_Y + row * CELL_SIZE + 1),
        static_cast<float>(CELL_SIZE - 2),
        static_cast<float>(CELL_SIZE - 2)
    };
}

bool GameRenderer::isPointInBoard(int x, int y, int& row, int& col) const {
    if (x < BOARD_OFFSET_X || y < BOARD_OFFSET_Y ||
        x >= BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE ||
        y >= BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE) {
        return false;
    }
    
    col = (x - BOARD_OFFSET_X) / CELL_SIZE;
    row = (y - BOARD_OFFSET_Y) / CELL_SIZE;
    
    return col >= 0 && col < BOARD_SIZE && row >= 0 && row < BOARD_SIZE;
}

// === BOARD RENDERING HELPERS ===
void GameRenderer::renderGrid() {
    SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
    
    // Draw vertical lines
    for (int i = 0; i <= BOARD_SIZE; i++) {
        const float x = static_cast<float>(BOARD_OFFSET_X + i * CELL_SIZE);
        const float y1 = static_cast<float>(BOARD_OFFSET_Y);
        const float y2 = static_cast<float>(BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE);
        SDL_RenderLine(renderer, x, y1, x, y2);
    }
    
    // Draw horizontal lines
    for (int i = 0; i <= BOARD_SIZE; i++) {
        const float y = static_cast<float>(BOARD_OFFSET_Y + i * CELL_SIZE);
        const float x1 = static_cast<float>(BOARD_OFFSET_X);
        const float x2 = static_cast<float>(BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE);
        SDL_RenderLine(renderer, x1, y, x2, y);
    }
}

void GameRenderer::renderSpecialSquares(const Board& board) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            SpecialSquare special = board.getSpecialSquare(row, col);
            if (special != SpecialSquare::NORMAL) {
                renderSingleSpecialSquare(row, col, special);
            }
        }
    }
}

void GameRenderer::renderSingleSpecialSquare(int row, int col, SpecialSquare special) {
    const float x = static_cast<float>(BOARD_OFFSET_X + col * CELL_SIZE);
    const float y = static_cast<float>(BOARD_OFFSET_Y + row * CELL_SIZE);
    
    // Fill special square with color
    const SDL_Color color = SPECIAL_SQUARE_COLORS[static_cast<int>(special)];
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    const SDL_FRect rect = {x + 1.0f, y + 1.0f, CELL_SIZE - 2.0f, CELL_SIZE - 2.0f};
    SDL_RenderFillRect(renderer, &rect);
    
    renderSpecialSquareText(x, y, special);
}

void GameRenderer::renderSpecialSquareText(float x, float y, SpecialSquare special) {
    const auto [label, textColor] = getSpecialSquareTextInfo(special);
    if (!label.empty()) {
        renderText(label, x + TEXT_OFFSET_X, y + TEXT_OFFSET_Y, textColor, smallFont);
    }
}

// === TILE RENDERING HELPERS ===
void GameRenderer::renderTiles(const Board& board) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                const SDL_FRect cellRect = getBoardCellRect(row, col);
                renderTile(cellRect.x, cellRect.y, tile);
            }
        }
    }
}

void GameRenderer::renderTile(float x, float y, const Tile* tile) {
    if (!tile) return;
    
    const SDL_FRect tileRect = {x + 2.0f, y + 2.0f, CELL_SIZE - 4.0f, CELL_SIZE - 4.0f};
    
    // Draw tile background
    SDL_SetRenderDrawColor(renderer, TILE_COLOR.r, TILE_COLOR.g, TILE_COLOR.b, TILE_COLOR.a);
    SDL_RenderFillRect(renderer, &tileRect);
    
    // Draw tile border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &tileRect);
    
    // Render letter (large)
    const std::string letter(1, tile->getLetter());
    renderText(letter, x + 8.0f, y + 5.0f, BLACK_COLOR, font);
    
    // Render points (small, bottom right)
    const std::string points = std::to_string(tile->getPoints());
    renderText(points, x + 25.0f, y + 22.0f, BLACK_COLOR, smallFont);
}

void GameRenderer::renderPlayerRack(const Player& player, float x, float y, bool isActive) {
    const std::vector<Tile>& rack = player.getRack();
    
    if (isActive) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
        const SDL_FRect highlightRect = {
            x - 5.0f, y - 5.0f, 
            static_cast<float>(rack.size()) * TILE_SPACING + 10.0f, 
            50.0f
        };
        SDL_RenderFillRect(renderer, &highlightRect);
    }
    
    // Clear the rack area first to remove old tiles
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_FRect clearRect = {
        x - 5.0f, y - 5.0f,
        7.0f * TILE_SPACING + 10.0f,  // Max 7 tiles
        50.0f
    };
    SDL_RenderFillRect(renderer, &clearRect);
    
    // Render each tile in the current rack
    for (size_t i = 0; i < rack.size(); i++) {
        const float tileX = x + static_cast<float>(i) * TILE_SPACING;
        renderTile(tileX, y, &rack[i]);
    }
}

// === TEXT RENDERING HELPERS ===
void GameRenderer::renderText(const std::string& text, float x, float y, const SDL_Color& color, TTF_Font* useFont) const {
    if (!useFont) useFont = font;
    if (!useFont) return;

    SDL_Surface* textSurface = TTF_RenderText_Solid(useFont, text.c_str(), 0, color);
    if (!textSurface) return;

    TextureRAII textTexture(SDL_CreateTextureFromSurface(renderer, textSurface));
    SDL_DestroySurface(textSurface);

    if (!textTexture) return;

    float textW, textH;
    SDL_GetTextureSize(textTexture.get(), &textW, &textH);

    const SDL_FRect destRect = {x, y, textW, textH};
    SDL_RenderTexture(renderer, textTexture.get(), nullptr, &destRect);
}

void GameRenderer::renderSpecialSquareLabel(int row, int col, const std::string& label, const SDL_Color& color) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || !smallFont) {
        return;
    }
    
    const float cellX = static_cast<float>(BOARD_OFFSET_X + col * CELL_SIZE);
    const float cellY = static_cast<float>(BOARD_OFFSET_Y + row * CELL_SIZE);
    
    int textW, textH;
    if (TTF_GetStringSize(smallFont, label.c_str(), 0, &textW, &textH) == 0) {
        const float textX = cellX + (CELL_SIZE - textW) / 2.0f;
        const float textY = cellY + (CELL_SIZE - textH) / 2.0f;
        renderText(label, textX, textY, color, smallFont);
    } else {
        renderText(label, cellX + TEXT_OFFSET_X, cellY + TEXT_OFFSET_Y, color, smallFont);
    }
}

// === MENU RENDERING HELPERS ===
void GameRenderer::renderMenuBackground() const {
    SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255);
    const SDL_FRect menuRect = {200.0f, 150.0f, MENU_WIDTH, MENU_HEIGHT};
    SDL_RenderFillRect(renderer, &menuRect);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &menuRect);
}

void GameRenderer::renderMenuContent() const {
    renderText("SCRABBLE GAME", 350.0f, 180.0f, WHITE_COLOR, font);
    renderMenuInstructions();
    renderText("Press any key to start playing!", 280.0f, 420.0f, WHITE_COLOR, font);
}

void GameRenderer::renderMenuInstructions() const {
    const std::vector<std::pair<std::string, float>> instructions = {
        {"Controls:", 220.0f},
        {"H - Show help", 250.0f},
        {"Mouse Click - Place tiles", 270.0f},
        {"ENTER - Confirm word", 290.0f},
        {"BACKSPACE - Cancel word", 310.0f},
        {"1-4 - Test functions", 330.0f},
        {"SPACE - Skip turn", 350.0f},
        {"ESC - Pause/Quit", 370.0f}
    };
    
    for (const auto& [text, y] : instructions) {
        TTF_Font* textFont = (text == "Controls:") ? font : smallFont;
        renderText(text, 220.0f, y, WHITE_COLOR, textFont);
    }
}

// === FONT MANAGEMENT HELPERS ===
bool GameRenderer::initializeTTF() {
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool GameRenderer::loadFontsFromPaths() {
    const std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/arial.ttf",
        "assets/fonts/arial.ttf",
        "arial.ttf"
    };
    
    for (const auto& path : fontPaths) {
        if (tryLoadFont(path)) {
            std::cout << "Fonts loaded from: " << path << std::endl;
            return true;
        }
    }
    
    std::cerr << "Font loading error: " << SDL_GetError() << std::endl;
    return false;
}

bool GameRenderer::tryLoadFont(const std::string& path) {
    font = TTF_OpenFont(path.c_str(), NORMAL_FONT_SIZE);
    smallFont = TTF_OpenFont(path.c_str(), SMALL_FONT_SIZE);
    
    if (font && smallFont) {
        return true;
    }
    
    cleanupFailedFontLoad();
    return false;
}

void GameRenderer::cleanupFailedFontLoad() {
    if (font) { TTF_CloseFont(font); font = nullptr; }
    if (smallFont) { TTF_CloseFont(smallFont); smallFont = nullptr; }
}

// === UTILITY HELPERS ===
std::pair<std::string, SDL_Color> GameRenderer::getSpecialSquareTextInfo(SpecialSquare special) const {
    switch (special) {
        case SpecialSquare::DOUBLE_LETTER: return {"2L", BLACK_COLOR};
        case SpecialSquare::TRIPLE_LETTER: return {"3L", WHITE_COLOR};
        case SpecialSquare::DOUBLE_WORD: return {"2W", BLACK_COLOR};
        case SpecialSquare::TRIPLE_WORD: return {"3W", WHITE_COLOR};
        case SpecialSquare::CENTER: return {"*", BLACK_COLOR};
        default: return {"", BLACK_COLOR};
    }
}

// === PRIVATE HELPER METHOD FOR PLAYER SCORE ===
void GameRenderer::renderPlayerScore(const Player& player, const SDL_FRect& scoreRect) const {
    // Background
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &scoreRect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &scoreRect);
    
    // Player info
    renderText(player.getName(), scoreRect.x + 10.0f, scoreRect.y + 5.0f, BLACK_COLOR, font);
    const std::string scoreText = "Score: " + std::to_string(player.getScore());
    renderText(scoreText, scoreRect.x + 10.0f, scoreRect.y + 25.0f, BLACK_COLOR, smallFont);
    const std::string tilesText = "Tiles: " + std::to_string(player.getRack().size());
    renderText(tilesText, scoreRect.x + 10.0f, scoreRect.y + 45.0f, BLACK_COLOR, smallFont);
}
