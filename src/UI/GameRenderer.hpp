#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>
#include <utility>
#include "../Core/Game.hpp"
#include "../Core/Board.hpp"
#include "../Core/Player.hpp"
#include "../Core/Tile.hpp"

// Add these enums for better type safety
enum class RenderLayer : int {
    BACKGROUND = 0,
    BOARD = 1,
    TILES = 2,
    UI = 3,
    OVERLAY = 4
};

enum class TextAlignment {
    LEFT,
    CENTER,
    RIGHT
};

class GameRenderer {
private:
    SDL_Renderer* renderer;
    SDL_Window* window;
    TTF_Font* font;
    TTF_Font* smallFont;
    
    // Colors and styling constants
    static const SDL_Color BOARD_COLOR;
    static const SDL_Color TILE_COLOR;
    static const SDL_Color TEXT_COLOR;
    static const SDL_Color SPECIAL_SQUARE_COLORS[];
    static const SDL_Color WHITE_COLOR;
    static const SDL_Color BLACK_COLOR;
    static const SDL_Color YELLOW_COLOR;
    
    // Board rendering constants
    static const int BOARD_OFFSET_X = 50;
    static const int BOARD_OFFSET_Y = 90;
    static const int CELL_SIZE = 40;
    static const int BOARD_SIZE = 15;

    // Magic numbers
    static constexpr float TILE_SPACING = 45.0f;
    static constexpr float RACK_PADDING = 20.0f;
    static constexpr float SCORE_WIDTH = 180.0f;
    static constexpr float SCORE_HEIGHT = 80.0f;
    static constexpr float TEXT_OFFSET_X = 8.0f;
    static constexpr float TEXT_OFFSET_Y = 12.0f;
    static constexpr float MENU_WIDTH = 600.0f;
    static constexpr float MENU_HEIGHT = 400.0f;

    // === HELPER CLASSES ===
    class TextureRAII {
    private:
        SDL_Texture* texture;
    public:
        explicit TextureRAII(SDL_Texture* tex) : texture(tex) {}
        ~TextureRAII() { if (texture) SDL_DestroyTexture(texture); }
        SDL_Texture* get() const { return texture; }
        explicit operator bool() const { return texture != nullptr; }
        
        // Prevent copying
        TextureRAII(const TextureRAII&) = delete;
        TextureRAII& operator=(const TextureRAII&) = delete;
        
        // Allow moving
        TextureRAII(TextureRAII&& other) noexcept : texture(other.texture) {
            other.texture = nullptr;
        }
        TextureRAII& operator=(TextureRAII&& other) noexcept {
            if (this != &other) {
                if (texture) SDL_DestroyTexture(texture);
                texture = other.texture;
                other.texture = nullptr;
            }
            return *this;
        }
    };
    
public:
    GameRenderer(SDL_Renderer* renderer, SDL_Window* window);
    ~GameRenderer();

    // === INITIALIZATION ===
    bool initializeFonts();
    void cleanupFonts();
    
    // === MAIN RENDERING ===
    void renderGameState(const Game& game);
    void renderBoard(const Board& board);
    void renderPickedTiles(const Game& game); 
    void renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer);
    void renderScores(const Player& player1, const Player& player2);
    
    // === SCREEN RENDERING ===
    void renderMenu();
    void renderGameOver(const Player& player1, const Player& player2);
    void renderPauseScreen();
    
    // === UTILITY ===
    void clear();
    void present();
    SDL_FRect getBoardCellRect(int row, int col) const;
    bool isPointInBoard(int x, int y, int& row, int& col) const;


private:
    // === BOARD RENDERING HELPERS ===
    void renderGrid();
    void renderSpecialSquares(const Board& board);
    void renderSingleSpecialSquare(int row, int col, SpecialSquare special);
    void renderSpecialSquareText(float x, float y, SpecialSquare special);
    
    // === TILE RENDERING HELPERS ===
    void renderTiles(const Board& board);
    void renderTile(float x, float y, const Tile* tile);
    void renderPlayerRack(const Player& player, float x, float y, bool isActive);
    
    // === TEXT RENDERING HELPERS ===
    void renderText(const std::string& text, float x, float y, const SDL_Color& color, TTF_Font* useFont = nullptr) const;
    void renderSpecialSquareLabel(int row, int col, const std::string& label, const SDL_Color& color) const;
    
    // === MENU RENDERING HELPERS ===
    void renderMenuBackground() const;
    void renderMenuContent() const;
    void renderMenuInstructions() const;
    
    // === FONT MANAGEMENT HELPERS ===
    bool initializeTTF();
    bool loadFontsFromPaths();
    bool tryLoadFont(const std::string& path);
    void cleanupFailedFontLoad();
    
    void renderPlayerScore(const Player& player, const SDL_FRect& scoreRect) const;

    // === UTILITY HELPERS ===
    std::pair<std::string, SDL_Color> getSpecialSquareTextInfo(SpecialSquare special) const;
};