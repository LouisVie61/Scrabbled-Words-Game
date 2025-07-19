#include "GameRenderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>

// === STATIC COLOR DEFINITIONS ===
const SDL_Color GameRenderer::BOARD_COLOR = {139, 69, 19, 255};      // Brown
const SDL_Color GameRenderer::TILE_COLOR = {255, 248, 220, 255};     // Cornsilk
const SDL_Color GameRenderer::TEXT_COLOR = {0, 0, 0, 255};           // Black
const SDL_Color GameRenderer::WHITE_COLOR = {255, 255, 255, 255};    // White
const SDL_Color GameRenderer::BLACK_COLOR = {0, 0, 0, 255};          // Black
const SDL_Color GameRenderer::YELLOW_COLOR = {255, 255, 0, 150};     // Yellow (transparent)
const SDL_Color GameRenderer::BLUE_COLOR = {100, 149, 237, 255};     // Cornflower Blue
const SDL_Color GameRenderer::GREEN_COLOR = {144, 238, 144, 255};    // Light Green
const SDL_Color GameRenderer::RED_COLOR = {220, 20, 60, 255};        // Crimson

const SDL_Color GameRenderer::SPECIAL_SQUARE_COLORS[] = {
    {255, 255, 255, 255},    // NORMAL - White
    {200, 230, 255, 255},    // DOUBLE_LETTER - Lighter Blue  
    {100, 180, 255, 255},    // TRIPLE_LETTER - Medium Blue
    {255, 200, 220, 255},    // DOUBLE_WORD - Lighter Pink
    {255, 150, 150, 255},    // TRIPLE_WORD - Lighter Red
    {255, 235, 100, 255}     // CENTER - Lighter Gold
};

// Define font sizes
static const int TITLE_FONT_SIZE = 24;
static const int NORMAL_FONT_SIZE = 18;
static const int SMALL_FONT_SIZE = 14;

// === CONSTRUCTOR & DESTRUCTOR ===
GameRenderer::GameRenderer(SDL_Renderer* renderer, SDL_Window* window) 
    : renderer(renderer), window(window), font(nullptr), smallFont(nullptr), titleFont(nullptr) {
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
        smallFont = nullptr;
    }
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    TTF_Quit();
}

// === MAIN RENDERING METHODS ===
void GameRenderer::renderGameState(const Game& game) {
    clear();    
    renderBoard(game.getBoard());
    renderPickedTiles(game);
    renderSelectedTileIndicator(game);
    renderPlayerRacks(game.getPlayer1(), game.getPlayer2(), game.getCurrentPlayerIndex());
    renderPlayerInfo(game.getPlayer1(), game.getPlayer2(), game.getCurrentPlayerIndex());
    renderCurrentWordScore(game);
    renderPauseButton();
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

void GameRenderer::renderSelectedTileIndicator(const Game& game) {
    if (game.getGameState() != GameState::PLAYING && game.getGameState() != GameState::PLACING_TILES) {
        return;
    }
    
    const Player& currentPlayer = (game.getCurrentPlayerIndex() == 0) ? game.getPlayer1() : game.getPlayer2();
    const auto& rack = currentPlayer.getRack();
    int selectedIndex = game.getSelectedTileIndex();
    
    if (rack.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(rack.size())) {
        return;
    }
    
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    const float rackY = BOARD_OFFSET_Y + boardHeight + 50.0f;
    const float totalRackWidth = 7.0f * TILE_SPACING;
    
    float rackStartX = (WINDOW_WIDTH - totalRackWidth) / 2.0f;
    
    if (rackStartX < 30.0f) {
        rackStartX = 30.0f;
    } else if (rackStartX + totalRackWidth > WINDOW_WIDTH - 30.0f) {
        rackStartX = WINDOW_WIDTH - totalRackWidth - 30.0f;
    }
    
    const float actualRackWidth = static_cast<float>(rack.size()) * TILE_SPACING;
    const float centerOffset = (totalRackWidth - actualRackWidth) / 2.0f;
    
    const float selectedTileX = rackStartX + centerOffset + static_cast<float>(selectedIndex) * TILE_SPACING;
    
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180); // Bright gold
    for (int i = 0; i < 3; i++) {
        const SDL_FRect glowRect = {
            selectedTileX - 3.0f - i, rackY - 3.0f - i,
            CELL_SIZE + 6.0f + 2*i, CELL_SIZE + 6.0f + 2*i
        };
        SDL_RenderRect(renderer, &glowRect);
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255); // Dark orange
    for (int i = 0; i < 2; i++) {
        const SDL_FRect innerBorder = {
            selectedTileX - 1.0f - i, rackY - 1.0f - i,
            CELL_SIZE + 2.0f + 2*i, CELL_SIZE + 2.0f + 2*i
        };
        SDL_RenderRect(renderer, &innerBorder);
    }
    
    const float tileCenterX = selectedTileX + (CELL_SIZE / 2.0f);
    float textX = tileCenterX - 40.0f;
    float textY = rackY - 40.0f;
    
    if (textX < 10.0f) {
        textX = 10.0f;
    } else if (textX + 80.0f > WINDOW_WIDTH - 10.0f) {
        textX = WINDOW_WIDTH - 90.0f;
    }

    int textW, textH;
    if (font && TTF_GetStringSize(font, "SELECTED", 0, &textW, &textH) == 0) {
    } else {
        // Fallback dimensions
        textW = 85;
        textH = 25;
    }
    
    const float bgWidth = static_cast<float>(textW) + 10.0f;  // Text width + padding
    const float bgHeight = static_cast<float>(textH) + 6.0f;  // Text height + padding
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220); // Semi-transparent white background
    const SDL_FRect textBgRect = {textX - 5.0f, textY - 3.0f, bgWidth, bgHeight};
    SDL_RenderFillRect(renderer, &textBgRect);
    
    // Red border
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderRect(renderer, &textBgRect);
    
    float centeredTextX = textBgRect.x + (bgWidth - textW) / 2.0f;
    float centeredTextY = textBgRect.y + (bgHeight - textH) / 2.0f;
    
    renderText("SELECTED", centeredTextX, centeredTextY, RED_COLOR, font);
    
    SDL_SetRenderDrawColor(renderer, RED_COLOR.r, RED_COLOR.g, RED_COLOR.b, RED_COLOR.a);
    const SDL_FRect underlineRect = {textX, textY + 18.0f, 80.0f, 2.0f};
    SDL_RenderFillRect(renderer, &underlineRect);
    
    const float arrowY = rackY - 12.0f;           // Arrow tip position
    const float arrowLength = 8.0f;              // Arrow shaft length
    const float arrowWidth = 6.0f;               // Arrow head width
    
    // Set arrow color
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Bright red
    
    // Draw arrow shaft (vertical line pointing down)
    SDL_RenderLine(renderer, 
                   tileCenterX, arrowY - arrowLength,   // Start point (top)
                   tileCenterX, arrowY);                // End point (tip)
    
    // Draw left arrow head line
    SDL_RenderLine(renderer, 
                   tileCenterX, arrowY,                 // Arrow tip
                   tileCenterX - arrowWidth, arrowY - arrowWidth); // Left wing
    
    // Draw right arrow head line  
    SDL_RenderLine(renderer, 
                   tileCenterX, arrowY,                 // Arrow tip
                   tileCenterX + arrowWidth, arrowY - arrowWidth); // Right wing
}

void GameRenderer::renderTilePreview(const Game& game, int mouseX, int mouseY) {
    if (game.getGameState() != GameState::PLAYING && game.getGameState() != GameState::PLACING_TILES) {
        return;
    }
    
    // Check if mouse is over board
    int row, col;
    if (!isPointInBoard(mouseX, mouseY, row, col)) {
        return;
    }
    
    // Check if cell is already occupied
    if (game.getBoard().getTile(row, col) != nullptr) {
        return;
    }
    
    // Get current player and selected tile
    const Player& currentPlayer = (game.getCurrentPlayerIndex() == 0) ? game.getPlayer1() : game.getPlayer2();
    const auto& rack = currentPlayer.getRack();
    int selectedIndex = game.getSelectedTileIndex();
    
    if (rack.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(rack.size())) {
        return;
    }
    
    // Get the cell position
    const SDL_FRect cellRect = getBoardCellRect(row, col);
    
    // Draw preview background (semi-transparent)
    SDL_SetRenderDrawColor(renderer, 200, 255, 200, 120); // Light green with transparency
    SDL_RenderFillRect(renderer, &cellRect);
    
    // Draw preview border
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 180); // Green border
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            cellRect.x - i, cellRect.y - i,
            cellRect.w + 2*i, cellRect.h + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // Draw preview tile (slightly transparent)
    const Tile& selectedTile = rack[selectedIndex];
    
    // Tile background (semi-transparent)
    SDL_SetRenderDrawColor(renderer, TILE_COLOR.r, TILE_COLOR.g, TILE_COLOR.b, 180);
    const SDL_FRect tileRect = {cellRect.x + 4.0f, cellRect.y + 4.0f, cellRect.w - 8.0f, cellRect.h - 8.0f};
    SDL_RenderFillRect(renderer, &tileRect);
    
    // Tile border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderRect(renderer, &tileRect);
    
    // Letter and points (slightly faded)
    const std::string letter(1, selectedTile.getLetter());
    float letterX = cellRect.x + (cellRect.w / 2.0f) - 7.0f;
    float letterY = cellRect.y + 7.0f;
    
    // Create a faded color for preview text
    SDL_Color fadedColor = {TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 180};
    renderText(letter, letterX, letterY, fadedColor, font);
    
    const std::string points = std::to_string(selectedTile.getPoints());
    float pointsX = cellRect.x + cellRect.w - 15.0f;
    float pointsY = cellRect.y + cellRect.h - 18.0f;
    renderText(points, pointsX, pointsY, fadedColor, smallFont);
    
    // Add "PREVIEW" label
    renderText("PREVIEW", cellRect.x - 20.0f, cellRect.y - 20.0f, GREEN_COLOR, smallFont);
}

void GameRenderer::renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer) {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    
    const float rackY = BOARD_OFFSET_Y + boardHeight + 50.0f;
    const float totalRackWidth = 7.0f * TILE_SPACING;
    const float availableWidth = WINDOW_WIDTH - 2 * 30.0f;
    
    // Center the rack but ensure it fits within screen bounds
    float rackStartX = (WINDOW_WIDTH - totalRackWidth) / 2.0f;
    
    // Render current player's rack (highlighted)
    const Player& currentPlayerRef = (currentPlayer == 0) ? player1 : player2;
    renderPlayerRack(currentPlayerRef, rackStartX, rackY, true);
}

void GameRenderer::renderPlayerInfo(const Player& player1, const Player& player2, int currentPlayer) {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float rightSideX = BOARD_OFFSET_X + boardWidth + PLAYER_INFO_PADDING;
    
    // Player 1 info (top right)
    const SDL_FRect player1Rect = {
        rightSideX, 
        BOARD_OFFSET_Y, 
        PLAYER_INFO_WIDTH, 
        PLAYER_INFO_HEIGHT
    };
    renderPlayerInfoBox(player1, player1Rect, true, currentPlayer == 0);
    
    // Player 2 info (below player 1)
    const SDL_FRect player2Rect = {
        rightSideX, 
        BOARD_OFFSET_Y + PLAYER_INFO_HEIGHT + PLAYER_INFO_PADDING, 
        PLAYER_INFO_WIDTH, 
        PLAYER_INFO_HEIGHT
    };
    renderPlayerInfoBox(player2, player2Rect, true, currentPlayer == 1);
}

void GameRenderer::renderCurrentWordScore(const Game& game) {
    const auto& currentWord = game.getCurrentWord();
    if (currentWord.empty()) return;
    
    // Calculate the score for current word being formed
    int currentWordScore = 0;
    std::string wordText = "";
    
    // Build the word string and calculate score
    for (const auto& placement : currentWord) {
        if (placement.tile) {
            wordText += placement.tile->getLetter();
            currentWordScore += placement.tile->getPoints();
        }
    }
    
    if (!wordText.empty()) {
        // Position score display below the rack area
        const float boardWidth = BOARD_SIZE * CELL_SIZE;
        const float boardHeight = BOARD_SIZE * CELL_SIZE;
        const float scoreY = BOARD_OFFSET_Y + boardHeight + RACK_PADDING + 100.0f;
        const float scoreX = BOARD_OFFSET_X + (boardWidth / 2) - 100.0f;
        
        // Background for score display
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 200);
        const SDL_FRect scoreRect = {scoreX, scoreY, 200.0f, SCORE_SECTION_HEIGHT};
        SDL_RenderFillRect(renderer, &scoreRect);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderRect(renderer, &scoreRect);
        
        // Current word text
        renderText("Current Word:", scoreX + 10.0f, scoreY + 10.0f, BLACK_COLOR, smallFont);
        renderText(wordText, scoreX + 10.0f, scoreY + 25.0f, BLUE_COLOR, font);
        
        // Current score
        const std::string scoreText = "Points: " + std::to_string(currentWordScore);
        renderText(scoreText, scoreX + 10.0f, scoreY + 40.0f, GREEN_COLOR, smallFont);
    }
}

void GameRenderer::renderScores(const Player& player1, const Player& player2) {
    // This method can be simplified since we now have renderPlayerInfo
    // which includes score information. For now, make it empty or call renderPlayerInfo
    // Since scores are already rendered in renderPlayerInfo, we can leave this empty
    // or use it for additional score displays if needed
}

// === SCREEN RENDERING METHODS ===
void GameRenderer::renderMenu() {
    renderMenuBackground();
    renderMenuContent();
}

void GameRenderer::renderPauseButton() {
    const float buttonX = WINDOW_WIDTH - PAUSE_BUTTON_SIZE - PAUSE_BUTTON_MARGIN;
    const float buttonY = PAUSE_BUTTON_MARGIN;
    
    // Enhanced button background with gradient effect
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 220);
    const SDL_FRect buttonRect = {buttonX, buttonY, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE};
    SDL_RenderFillRect(renderer, &buttonRect);
    
    // Button border - thicker and more visible
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            buttonX - i, buttonY - i,
            PAUSE_BUTTON_SIZE + 2*i, PAUSE_BUTTON_SIZE + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // Pause symbol (two vertical bars) - larger and better positioned
    SDL_SetRenderDrawColor(renderer, WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, WHITE_COLOR.a);
    const SDL_FRect bar1 = {buttonX + 10, buttonY + 6, 8, 28};
    const SDL_FRect bar2 = {buttonX + 22, buttonY + 6, 8, 28};
    SDL_RenderFillRect(renderer, &bar1);
    SDL_RenderFillRect(renderer, &bar2);
    
    // Add "PAUSE" label below button
    renderText("PAUSE", buttonX - 5, buttonY + PAUSE_BUTTON_SIZE + 5, BLACK_COLOR, smallFont);
}

// FIXED: Prevent flickering in pause menu
void GameRenderer::renderPauseMenu() {
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    const SDL_FRect overlay = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Menu background with better styling
    const float menuX = (WINDOW_WIDTH - PAUSE_MENU_WIDTH) / 2.0f;
    const float menuY = (WINDOW_HEIGHT - PAUSE_MENU_HEIGHT) / 2.0f;
    
    // Drop shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    const SDL_FRect shadowRect = {menuX + 5, menuY + 5, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT};
    SDL_RenderFillRect(renderer, &shadowRect);
    
    // Main menu background
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
    const SDL_FRect menuRect = {menuX, menuY, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT};
    SDL_RenderFillRect(renderer, &menuRect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int i = 0; i < 3; i++) {
        const SDL_FRect borderRect = {
            menuX - i, menuY - i,
            PAUSE_MENU_WIDTH + 2*i, PAUSE_MENU_HEIGHT + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // Menu title with better positioning
    renderText("GAME PAUSED", menuX + 80, menuY + 25, BLACK_COLOR, titleFont);
    
    // Menu options with proper spacing
    const std::vector<std::pair<std::string, float>> options = {
        {"Continue", menuY + 70},
        {"Surrender", menuY + 100},
        {"New Game", menuY + 130},
        {"Quit", menuY + 160}
    };
    
    for (const auto& [text, y] : options) {
        renderText(text, menuX + 90, y, BLACK_COLOR, font);
    }
}

bool GameRenderer::isPointInPauseButton(int x, int y) const {
    const float buttonX = WINDOW_WIDTH - PAUSE_BUTTON_SIZE - PAUSE_BUTTON_MARGIN;
    const float buttonY = PAUSE_BUTTON_MARGIN;
    
    return x >= static_cast<int>(buttonX) && x <= static_cast<int>(buttonX + PAUSE_BUTTON_SIZE) &&
           y >= static_cast<int>(buttonY) && y <= static_cast<int>(buttonY + PAUSE_BUTTON_SIZE);
}

PauseMenuOption GameRenderer::getPauseMenuOption(int x, int y) const {
    const float menuX = (WINDOW_WIDTH - PAUSE_MENU_WIDTH) / 2.0f;
    const float menuY = (WINDOW_HEIGHT - PAUSE_MENU_HEIGHT) / 2.0f;
    
    if (x < menuX || x > menuX + PAUSE_MENU_WIDTH || y < menuY || y > menuY + PAUSE_MENU_HEIGHT) {
        return static_cast<PauseMenuOption>(-1); // Invalid
    }
    
    const float relativeY = y - menuY;
    if (relativeY >= 70 && relativeY < 100) return PauseMenuOption::CONTINUE;
    if (relativeY >= 100 && relativeY < 130) return PauseMenuOption::SURRENDER;
    if (relativeY >= 130 && relativeY < 160) return PauseMenuOption::NEW_GAME;
    if (relativeY >= 160 && relativeY < 190) return PauseMenuOption::QUIT;
    
    return static_cast<PauseMenuOption>(-1); // Invalid
}

void GameRenderer::renderGameOver(const Player& player1, const Player& player2) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    const SDL_FRect overlay = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_RenderFillRect(renderer, &overlay);
    static Uint64 startTime = 0;
    if (startTime == 0) {
        startTime = SDL_GetTicks();
    }
    Uint64 currentTime = SDL_GetTicks();
    float elapsedTime = (currentTime - startTime) / 1000.0f;
    
    const float TITLE_ANIMATION_DURATION = 2.0f;
    const float SCOREBOARD_DELAY = 1.5f;
    const float centerX = WINDOW_WIDTH / 2.0f;
    
    if (elapsedTime < TITLE_ANIMATION_DURATION) {
        float animProgress = elapsedTime / TITLE_ANIMATION_DURATION;
        float bounceHeight = 50.0f * (1.0f - animProgress) * sin(animProgress * 10.0f);
        float sizeMultiplier = 2.0f - (1.0f * animProgress);
        float startY = WINDOW_HEIGHT / 2.0f - 50.0f;
        float endY = 80.0f;
        float currentY = startY + (endY - startY) * animProgress + bounceHeight;
        
        int baseTextW = 200, baseTextH = 30;
        if (titleFont && TTF_GetStringSize(titleFont, "GAME OVER", 0, &baseTextW, &baseTextH) != 0) {
            baseTextW = 200;
            baseTextH = 30;
        }
        
        float scaledW = baseTextW * sizeMultiplier;
        float scaledH = baseTextH * sizeMultiplier;
        const float centeredX = centerX - (scaledW / 2.0f);

        SDL_Color shadowColor = {80, 80, 80, static_cast<Uint8>(255 * (1.0f - animProgress * 0.3f))};
        SDL_Color mainColor = {RED_COLOR.r, RED_COLOR.g, RED_COLOR.b, static_cast<Uint8>(255 * (1.0f - animProgress * 0.2f))};

        float biggerSizeMultiplier = sizeMultiplier * 2.0f;
        int layerCount = static_cast<int>(biggerSizeMultiplier * 1.5f);

        for (int i = 0; i < layerCount; i++) {
            renderText("GAME OVER", centeredX + 3 + i, currentY + 3 + i, shadowColor, titleFont);
        }
        for (int i = 0; i < layerCount; i++) {
            renderText("GAME OVER", centeredX + i, currentY + i, mainColor, titleFont);
        }
    } else {
        int textW = 200, textH = 30;
        if (titleFont && TTF_GetStringSize(titleFont, "GAME OVER", 0, &textW, &textH) != 0) {
            textW = 200;
            textH = 30;
        }
            
            const float finalX = centerX - (textW / 2.0f);
            const float finalY = 80.0f;
            renderText("GAME OVER", finalX + 3, finalY + 3, {80, 80, 80, 255}, titleFont);
            renderText("GAME OVER", finalX, finalY, RED_COLOR, titleFont);
    }
    
    if (elapsedTime > SCOREBOARD_DELAY) {
        float scoreboardProgress = (elapsedTime - SCOREBOARD_DELAY) / 1.0f;
        if (scoreboardProgress > 1.0f) scoreboardProgress = 1.0f;
        
        const float boxWidth = 500.0f;
        const float boxHeight = 350.0f;
        const float finalBoxX = centerX - (boxWidth / 2.0f);
        const float finalBoxY = 180.0f;
        float startBoxY = WINDOW_HEIGHT;
        float currentBoxY = startBoxY + (finalBoxY - startBoxY) * scoreboardProgress;
        const SDL_FRect resultBox = {finalBoxX, currentBoxY, boxWidth, boxHeight};
        if (currentBoxY < WINDOW_HEIGHT) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(120 * scoreboardProgress));
            for (int i = 8; i >= 0; i--) {
                const SDL_FRect shadowLayer = {
                    finalBoxX + i, currentBoxY + i, 
                    boxWidth + (8 - i), boxHeight + (8 - i)
                };
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(15 * scoreboardProgress));
                SDL_RenderFillRect(renderer, &shadowLayer);
            }
            SDL_SetRenderDrawColor(renderer, WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, static_cast<Uint8>(240 * scoreboardProgress));
            SDL_RenderFillRect(renderer, &resultBox);
            const float cornerRadius = 10.0f;
            SDL_SetRenderDrawColor(renderer, WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, static_cast<Uint8>(240 * scoreboardProgress));
            for (int i = 0; i < cornerRadius; i++) {
                const SDL_FRect cornerRect = {
                    resultBox.x - i, resultBox.y - i,
                    cornerRadius + i, cornerRadius + i
                };
                SDL_RenderFillRect(renderer, &cornerRect);
            }
            
            for (int i = 0; i < 4; i++) {
                SDL_SetRenderDrawColor(renderer, 
                    BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, 
                    static_cast<Uint8>((200 - i * 30) * scoreboardProgress));
                const SDL_FRect borderRect = {
                    resultBox.x - i, resultBox.y - i,
                    resultBox.w + 2*i, resultBox.h + 2*i
                };
                SDL_RenderRect(renderer, &borderRect);
            }
            
            if (scoreboardProgress > 0.8f) {
                float contentAlpha = (scoreboardProgress - 0.8f) / 0.2f;
                
                // Determine winner text
                std::string winnerText;
                SDL_Color winnerColor = RED_COLOR;
                
                if (player1.getScore() > player2.getScore()) {
                    winnerText = player1.getName() + " WINS!";
                    winnerColor = BLUE_COLOR;
                } else if (player2.getScore() > player1.getScore()) {
                    winnerText = player2.getName() + " WINS!";
                    winnerColor = BLUE_COLOR;
                } else {
                    winnerText = "IT'S A TIE!";
                    winnerColor = BLUE_COLOR;
                }
                
                // Center the winner text
                int winnerW = 150, winnerH = 20;
                if (font && TTF_GetStringSize(font, winnerText.c_str(), 0, &winnerW, &winnerH) != 0) {
                    winnerW = 150;
                    winnerH = 20;
                }
                
                const float winnerX = resultBox.x + (resultBox.w - winnerW) / 2.0f;
                SDL_Color fadedWinnerColor = {winnerColor.r, winnerColor.g, winnerColor.b, static_cast<Uint8>(255 * contentAlpha)};
                renderText(winnerText, winnerX, resultBox.y + 40.0f, fadedWinnerColor, font);
                
                const float CONTENT_PADDING = 20.0f;
                float currentY = resultBox.y + 100.0f;
                
                SDL_Color fadedBlackColor = {BLACK_COLOR.r, BLACK_COLOR.g, BLACK_COLOR.b, static_cast<Uint8>(255 * contentAlpha)};
                SDL_Color fadedBlueColor = {BLUE_COLOR.r, BLUE_COLOR.g, BLUE_COLOR.b, static_cast<Uint8>(255 * contentAlpha)};
                SDL_Color fadedRedColor = {RED_COLOR.r, RED_COLOR.g, RED_COLOR.b, static_cast<Uint8>(255 * contentAlpha)};

                renderText("Final Scores:", resultBox.x + CONTENT_PADDING, currentY, fadedBlackColor, font);
                currentY += 30.0f;
                
                const std::string score1 = player1.getName() + ": " + std::to_string(player1.getScore());
                const std::string score2 = player2.getName() + ": " + std::to_string(player2.getScore());
                
                renderText(score1, resultBox.x + CONTENT_PADDING + 20.0f, currentY, fadedBlackColor, smallFont);
                currentY += 25.0f;
                renderText(score2, resultBox.x + CONTENT_PADDING + 20.0f, currentY, fadedBlackColor, smallFont);
                currentY += 40.0f;
                
                renderText("Controls:", resultBox.x + CONTENT_PADDING, currentY, fadedBlackColor, font);
                currentY += 30.0f;
                renderText("[R] Play Again", resultBox.x + CONTENT_PADDING + 20.0f, currentY, fadedBlueColor, smallFont);
                currentY += 25.0f;
                renderText("[ESC] Exit Game", resultBox.x + CONTENT_PADDING + 20.0f, currentY, fadedRedColor, smallFont);
            }
        }
    }
}

void GameRenderer::renderPauseScreen() {
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    const SDL_FRect overlay = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
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
    
    // Add bold border for special squares
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            x + 1.0f - i, y + 1.0f - i,
            CELL_SIZE - 2.0f + 2*i, CELL_SIZE - 2.0f + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    renderSpecialSquareText(x, y, special);
}

void GameRenderer::renderSpecialSquareText(float x, float y, SpecialSquare special) {
    const auto [label, textColor] = getSpecialSquareTextInfo(special);
    if (!label.empty()) {
        int textW, textH;
        if (smallFont && TTF_GetStringSize(smallFont, label.c_str(), 0, &textW, &textH) == 0) {
            // Center the text perfectly
            float textX = x + (CELL_SIZE - textW) / 2.0f;
            float textY = y + (CELL_SIZE - textH) / 2.0f;

            renderText(label, textX, textY, BLACK_COLOR, smallFont);
            renderText(label, textX + 1.0f, textY, BLACK_COLOR, smallFont);
            renderText(label, textX, textY + 1.0f, BLACK_COLOR, smallFont);
            renderText(label, textX + 1.0f, textY + 1.0f, BLACK_COLOR, smallFont);
        } else {
            // Fallback positioning with bold effect
            float textX = x + (CELL_SIZE / 2.0f) - 8.0f;
            float textY = y + (CELL_SIZE / 2.0f) - 6.0f;
            
            // Bold effect for fallback text
            renderText(label, textX, textY, BLACK_COLOR, smallFont);
            renderText(label, textX + 1.0f, textY, BLACK_COLOR, smallFont);
            renderText(label, textX, textY + 1.0f, BLACK_COLOR, smallFont);
            renderText(label, textX + 1.0f, textY + 1.0f, BLACK_COLOR, smallFont);
        }
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
    
    const float tileX = tileRect.x;
    const float tileY = tileRect.y;
    const float tileWidth = tileRect.w;
    const float tileHeight = tileRect.h;

    const std::string letter(1, tile->getLetter());
    
    int textW, textH;
    if (font && TTF_GetStringSize(font, letter.c_str(), 0, &textW, &textH) == 0) {
        // Position letter in upper left with small margin
        float letterX = tileX + 4.0f; // 4px margin from left edge
        float letterY = tileY + 2.0f; // 3px margin from top edge
        renderText(letter, letterX, letterY, BLACK_COLOR, font);
    } else {
        // Fallback positioning - upper left corner
        float letterX = tileX + 4.0f;
        float letterY = tileY + 2.0f;
        renderText(letter, letterX, letterY, BLACK_COLOR, font);
    }

    const std::string points = std::to_string(tile->getPoints());
    
    int pointsW, pointsH;
    if (smallFont && TTF_GetStringSize(smallFont, points.c_str(), 0, &pointsW, &pointsH) == 0) {
        float pointsX = x + CELL_SIZE - pointsW - 3.0f; // 3px margin from right edge
        float pointsY = y + CELL_SIZE - pointsH - 10.0f; // 2px margin from bottom edge
        renderText(points, pointsX, pointsY, BLACK_COLOR, smallFont);
    } else {
        float pointsX = x + CELL_SIZE - 12.0f;
        float pointsY = y + CELL_SIZE - 20.0f;
        renderText(points, pointsX, pointsY, BLACK_COLOR, smallFont);
    }
}

// FIXED: Improved rack rendering with better spacing
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
    
    // FIXED: Better spacing calculation for even distribution
    const float totalWidth = 7.0f * TILE_SPACING; // Fixed width for 7 tiles
    const float actualRackWidth = static_cast<float>(rack.size()) * TILE_SPACING;
    const float centerOffset = (totalWidth - actualRackWidth) / 2.0f;
    
    // Render each tile in the current rack with proper spacing
    for (size_t i = 0; i < rack.size(); i++) {
        const float tileX = x + centerOffset + static_cast<float>(i) * TILE_SPACING;
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
        "C:/Windows/Fonts/segoeui.ttf",      // Segoe UI (modern Windows font)
        "C:/Windows/Fonts/calibri.ttf",     // Calibri (clean and readable)
        "C:/Windows/Fonts/tahoma.ttf",      // Tahoma (fallback)
        "C:/Windows/Fonts/arial.ttf",       // Arial (last resort)
        "assets/fonts/segoeui.ttf",
        "segoeui.ttf"
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
    titleFont = TTF_OpenFont(path.c_str(), TITLE_FONT_SIZE);
    font = TTF_OpenFont(path.c_str(), NORMAL_FONT_SIZE);
    smallFont = TTF_OpenFont(path.c_str(), SMALL_FONT_SIZE);
    
    if (titleFont && font && smallFont) {
        return true;
    }
    
    cleanupFailedFontLoad();
    return false;
}

void GameRenderer::cleanupFailedFontLoad() {
    if (titleFont) { TTF_CloseFont(titleFont); titleFont = nullptr; }
    if (font) { TTF_CloseFont(font); font = nullptr; }
    if (smallFont) { TTF_CloseFont(smallFont); smallFont = nullptr; }
}

// === UTILITY HELPERS ===
std::pair<std::string, SDL_Color> GameRenderer::getSpecialSquareTextInfo(SpecialSquare special) const {
    switch (special) {
        case SpecialSquare::DOUBLE_LETTER: return {"2L", BLACK_COLOR};
        case SpecialSquare::TRIPLE_LETTER: return {"3L", BLACK_COLOR};
        case SpecialSquare::DOUBLE_WORD: return {"2W", BLACK_COLOR};
        case SpecialSquare::TRIPLE_WORD: return {"3W", BLACK_COLOR};
        case SpecialSquare::CENTER: return {"CT", BLACK_COLOR};
        default: return {"", BLACK_COLOR};
    }
}

// === PRIVATE HELPER METHOD FOR PLAYER INFO ===
void GameRenderer::renderPlayerInfoBox(const Player& player, const SDL_FRect& rect, bool isActive, bool isCurrentTurn) const {
    // Background color changes based on turn
    if (isCurrentTurn) {
        SDL_SetRenderDrawColor(renderer, GREEN_COLOR.r, GREEN_COLOR.g, GREEN_COLOR.b, 200);
    } else {
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    
    // Border - thicker and colored if current turn
    if (isCurrentTurn) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Dark green border
        for (int i = 0; i < 3; i++) {
            const SDL_FRect borderRect = {
                rect.x - i, rect.y - i, 
                rect.w + 2*i, rect.h + 2*i
            };
            SDL_RenderRect(renderer, &borderRect);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderRect(renderer, &rect);
    }
    
    const float TEXT_PADDING = 20.0f;
    
    renderText(player.getName(), rect.x + TEXT_PADDING, rect.y + 15.0f, BLACK_COLOR, font);
    
    if (isCurrentTurn) {
        renderText(">>> YOUR TURN", rect.x + TEXT_PADDING, rect.y + 35.0f, RED_COLOR, smallFont);
    }
    
    const std::string scoreText = "Score: " + std::to_string(player.getScore());
    renderText(scoreText, rect.x + TEXT_PADDING, rect.y + 55.0f, BLACK_COLOR, smallFont);
    
    const std::string tilesText = "Tiles: " + std::to_string(player.getRack().size());
    renderText(tilesText, rect.x + TEXT_PADDING, rect.y + 75.0f, BLACK_COLOR, smallFont);
    
    if (player.isAI()) {
        renderText("(AI)", rect.x + TEXT_PADDING, rect.y + 95.0f, BLUE_COLOR, smallFont);
    }
}