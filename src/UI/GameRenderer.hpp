#pragma once
#include <SDL3/SDL.h>
#include "../Core/Game.hpp"
#include "../Core/Board.hpp"
#include "../Core/Player.hpp"

class GameRenderer {
private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    
    // Colors and styling constants
    static const SDL_Color BOARD_COLOR;
    static const SDL_Color TILE_COLOR;
    static const SDL_Color TEXT_COLOR;
    static const SDL_Color SPECIAL_SQUARE_COLORS[];
    
    // Board rendering constants
    static const int BOARD_OFFSET_X = 50;
    static const int BOARD_OFFSET_Y = 50;
    static const int CELL_SIZE = 40;
    static const int BOARD_SIZE = 15;
    
public:
    GameRenderer(SDL_Renderer* renderer, SDL_Window* window);
    ~GameRenderer();
    
    // Main rendering methods
    void renderBoard(const Board& board);
    void renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer);
    void renderScores(const Player& player1, const Player& player2);
    void renderGameState(const Game& game);
    void renderMenu();
    void renderGameOver(const Player& player1, const Player& player2);
    void renderPauseScreen();
    
    // Helper rendering methods
    void renderGrid();
    void renderSpecialSquares(const Board& board);
    void renderTiles(const Board& board);
    void renderTile(float x, float y, const Tile* tile);  // Changed to float
    void renderPlayerRack(const Player& player, float x, float y, bool isActive);  // Changed to float
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    
    // Utility methods
    void clear();
    void present();
    SDL_FRect getBoardCellRect(int row, int col) const;  // Changed to SDL_FRect
    bool isPointInBoard(int x, int y, int& row, int& col) const;
};