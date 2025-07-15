#include "GameRenderer.hpp"
#include <iostream>

// Static color definitions
const SDL_Color GameRenderer::BOARD_COLOR = {139, 69, 19, 255};      // Brown
const SDL_Color GameRenderer::TILE_COLOR = {255, 248, 220, 255};     // Cornsilk
const SDL_Color GameRenderer::TEXT_COLOR = {0, 0, 0, 255};           // Black

const SDL_Color GameRenderer::SPECIAL_SQUARE_COLORS[] = {
    {255, 255, 255, 255},  // NORMAL - White
    {173, 216, 230, 255},  // DOUBLE_LETTER - Light Blue
    {0, 0, 255, 255},      // TRIPLE_LETTER - Blue
    {255, 192, 203, 255},  // DOUBLE_WORD - Pink
    {255, 0, 0, 255},      // TRIPLE_WORD - Red
    {255, 215, 0, 255}     // CENTER - Gold
};

GameRenderer::GameRenderer(SDL_Renderer* renderer, SDL_Window* window) 
    : renderer(renderer), window(window) {
}

GameRenderer::~GameRenderer() {
}

void GameRenderer::renderBoard(const Board& board) {
    renderGrid();
    renderSpecialSquares(board);
    renderTiles(board);
}

void GameRenderer::renderGrid() {
    // Set grid color (dark brown)
    SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
    
    // Draw vertical lines - SDL3 uses float coordinates
    for (int i = 0; i <= BOARD_SIZE; i++) {
        float x = static_cast<float>(BOARD_OFFSET_X + i * CELL_SIZE);
        float y1 = static_cast<float>(BOARD_OFFSET_Y);
        float y2 = static_cast<float>(BOARD_OFFSET_Y + BOARD_SIZE * CELL_SIZE);
        SDL_RenderLine(renderer, x, y1, x, y2);
    }
    
    // Draw horizontal lines
    for (int i = 0; i <= BOARD_SIZE; i++) {
        float y = static_cast<float>(BOARD_OFFSET_Y + i * CELL_SIZE);
        float x1 = static_cast<float>(BOARD_OFFSET_X);
        float x2 = static_cast<float>(BOARD_OFFSET_X + BOARD_SIZE * CELL_SIZE);
        SDL_RenderLine(renderer, x1, y, x2, y);
    }
}

void GameRenderer::renderSpecialSquares(const Board& board) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            SpecialSquare special = board.getSpecialSquare(row, col);
            
            if (special != SpecialSquare::NORMAL) {
                SDL_FRect cellRect = getBoardCellRect(row, col);
                
                // Get color based on special square type
                SDL_Color color = SPECIAL_SQUARE_COLORS[static_cast<int>(special)];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &cellRect);
            }
        }
    }
}

void GameRenderer::renderTiles(const Board& board) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                SDL_FRect cellRect = getBoardCellRect(row, col);
                renderTile(cellRect.x, cellRect.y, tile);
            }
        }
    }
}

void GameRenderer::renderTile(float x, float y, const Tile* tile) {
    if (!tile) return;
    
    SDL_FRect tileRect = {x + 2.0f, y + 2.0f, CELL_SIZE - 4.0f, CELL_SIZE - 4.0f};
    
    // Draw tile background
    SDL_SetRenderDrawColor(renderer, TILE_COLOR.r, TILE_COLOR.g, TILE_COLOR.b, TILE_COLOR.a);
    SDL_RenderFillRect(renderer, &tileRect);
    
    // Draw tile border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &tileRect);
    
    // TODO: Render letter and points on tile
    // This would require font rendering (SDL_ttf)
    // For now, we'll just draw the tile background
}

void GameRenderer::renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer) {
    float rackPadding = 20.0f;
    float tileSize = CELL_SIZE - 4.0f;
    float rackHeight = tileSize;
    float boardHeight = BOARD_SIZE * CELL_SIZE;

    // Bottom
    float bottomY = BOARD_OFFSET_Y + boardHeight + rackPadding;
    renderPlayerRack(player1, BOARD_OFFSET_X, bottomY, currentPlayer == 0);

    // Top
    float topY = BOARD_OFFSET_Y - rackHeight - rackPadding;
    renderPlayerRack(player2, BOARD_OFFSET_X, topY, currentPlayer == 1);
}

void GameRenderer::renderPlayerRack(const Player& player, float x, float y, bool isActive) {
    const auto& rack = player.getRack();
    
    // Highlight active player's rack
    if (isActive) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100); // Yellow highlight
        SDL_FRect highlightRect = {x - 5.0f, y - 5.0f, 
                                  static_cast<float>(rack.size()) * 45.0f + 10.0f, 50.0f};
        SDL_RenderFillRect(renderer, &highlightRect);
    }
    
    // Render each tile in the rack
    for (size_t i = 0; i < rack.size(); i++) {
        float tileX = x + static_cast<float>(i) * 45.0f;
        renderTile(tileX, y, &rack[i]);
    }
}

void GameRenderer::renderScores(const Player& player1, const Player& player2) {
    // Player 1 score area
    SDL_FRect score1Rect = {650.0f, 650.0f, 200.0f, 50.0f};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &score1Rect);
    
    // Player 2 score area
    SDL_FRect score2Rect = {650.0f, 100.0f, 200.0f, 50.0f};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &score2Rect);
}

void GameRenderer::renderGameState(const Game& game) {
    // TODO: Render current player turn, remaining tiles, etc.
    // This would show whose turn it is, tiles left in bag, etc.
}

void GameRenderer::renderMenu() {
    // Simple menu background
    SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255);
    SDL_FRect menuRect = {200.0f, 200.0f, 400.0f, 300.0f};
    SDL_RenderFillRect(renderer, &menuRect);
    
    // TODO: Add menu buttons and text
}

void GameRenderer::renderGameOver(const Player& player1, const Player& player2) {
    // Game over overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_FRect overlay = {0.0f, 0.0f, 1024.0f, 768.0f};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Game over box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect gameOverRect = {300.0f, 250.0f, 400.0f, 200.0f};
    SDL_RenderFillRect(renderer, &gameOverRect);
    
    // TODO: Add final scores and winner text
}

void GameRenderer::renderPauseScreen() {
    // Pause overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_FRect overlay = {0.0f, 0.0f, 1024.0f, 768.0f};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Pause text area
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect pauseRect = {400.0f, 350.0f, 200.0f, 100.0f};
    SDL_RenderFillRect(renderer, &pauseRect);
}

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