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
    TTF_Font* titleFont;
    TTF_Font* specialFont;
    
    static const SDL_Color BOARD_COLOR;
    static const SDL_Color TILE_COLOR;
    static const SDL_Color TEXT_COLOR;
    static const SDL_Color SPECIAL_SQUARE_COLORS[];
    static const SDL_Color WHITE_COLOR;
    static const SDL_Color BLACK_COLOR;
    static const SDL_Color YELLOW_COLOR;
    static const SDL_Color BLUE_COLOR;
    static const SDL_Color GREEN_COLOR;
    static const SDL_Color RED_COLOR;
    
    static const int WINDOW_WIDTH = 1200;
    static const int WINDOW_HEIGHT = 800;
    static const int BOARD_OFFSET_X = 150;
    static const int BOARD_OFFSET_Y = 80;
    static const int CELL_SIZE = 35;
    static const int BOARD_SIZE = 15;

    static constexpr float PLAYER_INFO_WIDTH = 180.0f;
    static constexpr float PLAYER_INFO_HEIGHT = 140.0f;
    static constexpr float PLAYER_INFO_PADDING = 20.0f;
    static constexpr float TILE_SPACING = 40.0f;
    static constexpr float RACK_PADDING = 15.0f;
    static constexpr float SCORE_SECTION_HEIGHT = 60.0f;
    static constexpr float TEXT_OFFSET_X = 6.0f;
    static constexpr float TEXT_OFFSET_Y = 10.0f;
    static constexpr float MENU_WIDTH = 600.0f;
    static constexpr float MENU_HEIGHT = 400.0f;
    
    static constexpr float PAUSE_BUTTON_SIZE = 40.0f;
    static constexpr float PAUSE_BUTTON_MARGIN = 15.0f;
    static constexpr float PAUSE_MENU_WIDTH = 280.0f;
    static constexpr float PAUSE_MENU_HEIGHT = 220.0f;

    class TextureRAII {
    private:
        SDL_Texture* texture;
    public:
        explicit TextureRAII(SDL_Texture* tex) : texture(tex) {}
        ~TextureRAII() { if (texture) SDL_DestroyTexture(texture); }
        SDL_Texture* get() const { return texture; }
        explicit operator bool() const { return texture != nullptr; }
        
        TextureRAII(const TextureRAII&) = delete;
        TextureRAII& operator=(const TextureRAII&) = delete;
        
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
    bool initializeFonts();
    void cleanupFonts();
    void renderGameState(const Game& game);
    void renderBoard(const Board& board);
    void renderPickedTiles(const Game& game); 
    void renderSelectedTileIndicator(const Game& game);
    void renderTilePreview(const Game& game, int mouseX, int mouseY);
    void renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer);
    void renderPlayerInfo(const Player& player1, const Player& player2, int currentPlayer);
    void renderScores(const Player& player1, const Player& player2);
    void renderCurrentWordScore(const Game& game);
    void renderGameStart();
    void renderMenu();
    void renderGameOver(const Player& player1, const Player& player2);
    void renderPauseScreen();
    void renderPauseButton();
    void renderPauseMenu();
    void clear();
    void present();
    SDL_FRect getBoardCellRect(int row, int col) const;
    bool isPointInBoard(int x, int y, int& row, int& col) const;
    bool isPointInPauseButton(int x, int y) const;
    PauseMenuOption getPauseMenuOption(int x, int y) const;
    bool isPointInStartButton(int x, int y) const;
    bool isPointInTutorialButton(int x, int y) const;
    bool isPointInExitButton(int x, int y) const;
    bool isPointInPlayAgainButton(int x, int y) const;
    bool isPointInMainMenuButton(int x, int y) const;
    bool isPointInGameOverExitButton(int x, int y) const;
    bool isPointInSwitchTurnButton(int x, int y) const;
    bool isPointInSubmitButton(int x, int y) const;
    bool isPointInCancelButton(int x, int y) const;
    static void toggleTutorial();
    static bool isTutorialVisible();

private:
    void renderGrid();
    void renderSpecialSquares(const Board& board);
    void renderSingleSpecialSquare(int row, int col, SpecialSquare special);
    void renderSpecialSquareText(float x, float y, SpecialSquare special);
    void renderTiles(const Board& board);
    void renderTile(float x, float y, const Tile* tile);
    void renderPlayerRack(const Player& player, float x, float y, bool isActive);
    void renderText(const std::string& text, float x, float y, const SDL_Color& color, TTF_Font* useFont = nullptr) const;
    void renderSpecialSquareLabel(int row, int col, const std::string& label, const SDL_Color& color) const;
    void renderMenuBackground() const;
    void renderMenuContent() const;
    void renderMenuInstructions() const;
    bool initializeTTF();
    bool loadFontsFromPaths();
    bool tryLoadFont(const std::string& path);
    void cleanupFailedFontLoad();
    void renderPlayerInfoBox(const Player& player, const SDL_FRect& rect, bool isActive, bool isCurrentTurn) const;
    void renderFullWidthTitle(float elapsedTime, float startY, float titleHeight);
    void renderInformationBoxes(float elapsedTime, float startX, float startY, float boxWidth);
    void renderSideButtons(float elapsedTime, float startX, float startY, float buttonWidth);
    void renderGameOverTitle(float elapsedTime, float startY, float titleHeight);
    void renderGameOverContent(float elapsedTime, float startX, float startY, float boxWidth, const Player& player1, const Player& player2);
    void renderGameOverButtons(float elapsedTime, float startX, float startY, float buttonWidth);
    void renderPauseTitle(float elapsedTime, float startY, float titleHeight);
    void renderPauseContent(float elapsedTime, float startX, float startY, float boxWidth);
    void renderPauseButtons(float elapsedTime, float startX, float startY, float buttonWidth);
    std::pair<std::string, SDL_Color> getSpecialSquareTextInfo(SpecialSquare special) const;
};