#include "GameRenderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

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

struct Particle {
    float x, y, vx, vy, life, maxLife;
    SDL_Color color;
};

static bool tutorialVisible = false;
static bool tutorialAnimating = false;
static float tutorialAnimationTimer = 0.0f;
static Uint64 tutorialAnimationStart = 0;
const float TUTORIAL_ANIMATION_DURATION = 0.5f; 


// Define font sizes
static const int SPECIAL_FONT_SIZE = 48;
static const int TITLE_FONT_SIZE = 22;
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
    if (specialFont) {
        TTF_CloseFont(specialFont);
        specialFont = nullptr;
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
    const auto& currentWord = game.getCurrentWord();
    
    for (const auto& placement : currentWord) {
        int row = placement.row;
        int col = placement.col;
        const Tile* tile = placement.tile;
        
        if (tile) {
            const SDL_FRect cellRect = getBoardCellRect(row, col);
            SDL_SetRenderDrawColor(renderer, YELLOW_COLOR.r, YELLOW_COLOR.g, YELLOW_COLOR.b, YELLOW_COLOR.a);
            const SDL_FRect highlightRect = {
                cellRect.x - 2.0f, cellRect.y - 2.0f, 
                cellRect.w + 4.0f, cellRect.h + 4.0f
            };
            SDL_RenderFillRect(renderer, &highlightRect);
            
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
}

void GameRenderer::renderTilePreview(const Game& game, int mouseX, int mouseY) {
    if (game.getGameState() != GameState::PLAYING && game.getGameState() != GameState::PLACING_TILES) {
        return;
    }
    
    int row, col;
    if (!isPointInBoard(mouseX, mouseY, row, col)) {
        return;
    }
    if (game.getBoard().getTile(row, col) != nullptr) {
        return;
    }
    
    const Player& currentPlayer = (game.getCurrentPlayerIndex() == 0) ? game.getPlayer1() : game.getPlayer2();
    const auto& rack = currentPlayer.getRack();
    int selectedIndex = game.getSelectedTileIndex();
    
    if (rack.empty() || selectedIndex < 0 || selectedIndex >= static_cast<int>(rack.size())) {
        return;
    }
    
    const SDL_FRect cellRect = getBoardCellRect(row, col);
    
    // Draw preview background (semi-transparent)
    SDL_SetRenderDrawColor(renderer, 200, 255, 200, 120);
    SDL_RenderFillRect(renderer, &cellRect);
    
    // Draw preview border
    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 180);
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            cellRect.x - i, cellRect.y - i,
            cellRect.w + 2*i, cellRect.h + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    const Tile& selectedTile = rack[selectedIndex];
    
    SDL_SetRenderDrawColor(renderer, TILE_COLOR.r, TILE_COLOR.g, TILE_COLOR.b, 180);
    const SDL_FRect tileRect = {cellRect.x + 4.0f, cellRect.y + 4.0f, cellRect.w - 8.0f, cellRect.h - 8.0f};
    SDL_RenderFillRect(renderer, &tileRect);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderRect(renderer, &tileRect);
    
    const std::string letter(1, selectedTile.getLetter());
    float letterX = cellRect.x + (cellRect.w / 2.0f) - 7.0f;
    float letterY = cellRect.y + 7.0f;
    
    SDL_Color fadedColor = {TEXT_COLOR.r, TEXT_COLOR.g, TEXT_COLOR.b, 180};
    renderText(letter, letterX, letterY, fadedColor, font);
    
    const std::string points = std::to_string(selectedTile.getPoints());
    float pointsX = cellRect.x + cellRect.w - 15.0f;
    float pointsY = cellRect.y + cellRect.h - 18.0f;
    renderText(points, pointsX, pointsY, fadedColor, smallFont);
    
    renderText("PREVIEW", cellRect.x - 20.0f, cellRect.y - 20.0f, GREEN_COLOR, smallFont);
}

void GameRenderer::renderPlayerRacks(const Player& player1, const Player& player2, int currentPlayer) {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float boardHeight = BOARD_SIZE * CELL_SIZE;
    
    const float rackY = BOARD_OFFSET_Y + boardHeight + 50.0f;
    const float totalRackWidth = 7.0f * TILE_SPACING;
    const float availableWidth = WINDOW_WIDTH - 2 * 30.0f;
    
    float rackStartX = (WINDOW_WIDTH - totalRackWidth) / 2.0f;
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

    // Render action buttons below player info boxes
    const float buttonStartY = BOARD_OFFSET_Y + 2 * PLAYER_INFO_HEIGHT + 2 * PLAYER_INFO_PADDING + 20.0f;
    const float buttonWidth = 55.0f;
    const float buttonHeight = 30.0f;
    const float buttonGap = 10.0f;
    

    // SWITCH TURN button
    const SDL_FRect switchButton = {
        rightSideX, 
        buttonStartY, 
        buttonWidth, 
        buttonHeight
    };
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255); // Blue
    SDL_RenderFillRect(renderer, &switchButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &switchButton);
    renderText("SWITCH", switchButton.x + 2, switchButton.y + 8, BLACK_COLOR, smallFont);
    
    // SUBMIT button
    const SDL_FRect submitButton = {
        rightSideX + buttonWidth + buttonGap, 
        buttonStartY, 
        buttonWidth, 
        buttonHeight
    };
    SDL_SetRenderDrawColor(renderer, 34, 197, 94, 255); // Green
    SDL_RenderFillRect(renderer, &submitButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &submitButton);
    renderText("SUBMIT", submitButton.x + 2, submitButton.y + 8, BLACK_COLOR, smallFont);
    
    // CANCEL button
    const SDL_FRect cancelButton = {
        rightSideX + 2 * (buttonWidth + buttonGap), 
        buttonStartY, 
        buttonWidth, 
        buttonHeight
    };
    SDL_SetRenderDrawColor(renderer, 239, 68, 68, 255); // Red
    SDL_RenderFillRect(renderer, &cancelButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &cancelButton);
    renderText("CANCEL", cancelButton.x + 2, cancelButton.y + 8, BLACK_COLOR, smallFont);
}

bool GameRenderer::isPointInSwitchTurnButton(int x, int y) const {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float rightSideX = BOARD_OFFSET_X + boardWidth + PLAYER_INFO_PADDING;
    const float buttonStartY = BOARD_OFFSET_Y + 2 * PLAYER_INFO_HEIGHT + 2 * PLAYER_INFO_PADDING + 20.0f;
    const float buttonWidth = 50.0f;
    const float buttonHeight = 30.0f;
    
    return (x >= rightSideX && x <= rightSideX + buttonWidth &&
            y >= buttonStartY && y <= buttonStartY + buttonHeight);
}

bool GameRenderer::isPointInSubmitButton(int x, int y) const {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float rightSideX = BOARD_OFFSET_X + boardWidth + PLAYER_INFO_PADDING;
    const float buttonStartY = BOARD_OFFSET_Y + 2 * PLAYER_INFO_HEIGHT + 2 * PLAYER_INFO_PADDING + 20.0f;
    const float buttonWidth = 50.0f;
    const float buttonHeight = 30.0f;
    const float buttonGap = 10.0f;
    
    const float submitButtonX = rightSideX + buttonWidth + buttonGap;
    
    return (x >= submitButtonX && x <= submitButtonX + buttonWidth &&
            y >= buttonStartY && y <= buttonStartY + buttonHeight);
}

bool GameRenderer::isPointInCancelButton(int x, int y) const {
    const float boardWidth = BOARD_SIZE * CELL_SIZE;
    const float rightSideX = BOARD_OFFSET_X + boardWidth + PLAYER_INFO_PADDING;
    const float buttonStartY = BOARD_OFFSET_Y + 2 * PLAYER_INFO_HEIGHT + 2 * PLAYER_INFO_PADDING + 20.0f;
    const float buttonWidth = 50.0f;
    const float buttonHeight = 30.0f;
    const float buttonGap = 10.0f;
    
    const float cancelButtonX = rightSideX + 2 * (buttonWidth + buttonGap);
    
    return (x >= cancelButtonX && x <= cancelButtonX + buttonWidth &&
            y >= buttonStartY && y <= buttonStartY + buttonHeight);
}

void GameRenderer::renderCurrentWordScore(const Game& game) {
    const auto& currentWord = game.getCurrentWord();
    if (currentWord.empty()) return;

    std::string completeWord = "";
    int totalScore = 0;
    int wordMultiplier = 1;
    
    std::vector<std::pair<int, int>> positions;
    for (const auto& placement : currentWord) {
        positions.push_back({placement.row, placement.col});
    }
    
    if (positions.empty()) return;
    

    std::sort(positions.begin(), positions.end(), 
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            if (a.first == b.first) return a.second < b.second;
            return a.first < b.first;
        });
    
    bool isHorizontal = (positions[0].first == positions.back().first);
    
    if (isHorizontal) {
        int row = positions[0].first;
        int startCol = positions[0].second;
        int endCol = positions.back().second;
        
        const Board& board = game.getBoard();
        while (startCol > 0 && board.getTile(row, startCol - 1) != nullptr) {
            startCol--;
        }
        while (endCol < 14 && board.getTile(row, endCol + 1) != nullptr) {
            endCol++;
        }
        
        for (int col = startCol; col <= endCol; col++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                completeWord += tile->getLetter();
                int letterPoints = tile->getPoints();
                
                bool isNewlyPlaced = false;
                for (const auto& placement : currentWord) {
                    if (placement.row == row && placement.col == col) {
                        isNewlyPlaced = true;
                        break;
                    }
                }
                
                if (isNewlyPlaced) {
                    SpecialSquare special = board.getSpecialSquare(row, col);
                    
                    switch (special) {
                        case SpecialSquare::DOUBLE_LETTER:
                            letterPoints *= 2;
                            break;
                        case SpecialSquare::TRIPLE_LETTER:
                            letterPoints *= 3;
                            break;
                        case SpecialSquare::DOUBLE_WORD:
                        case SpecialSquare::CENTER:
                            wordMultiplier *= 2;
                            break;
                        case SpecialSquare::TRIPLE_WORD:
                            wordMultiplier *= 3;
                            break;
                        default:
                            break;
                    }
                }
                
                totalScore += letterPoints;
            }
        }
    } else {
        int col = positions[0].second;
        int startRow = positions[0].first;
        int endRow = positions.back().first;
        
        const Board& board = game.getBoard();
        while (startRow > 0 && board.getTile(startRow - 1, col) != nullptr) {
            startRow--;
        }
        while (endRow < 14 && board.getTile(endRow + 1, col) != nullptr) {
            endRow++;
        }
        
        for (int row = startRow; row <= endRow; row++) {
            const Tile* tile = board.getTile(row, col);
            if (tile) {
                completeWord += tile->getLetter();
                int letterPoints = tile->getPoints();
                
                bool isNewlyPlaced = false;
                for (const auto& placement : currentWord) {
                    if (placement.row == row && placement.col == col) {
                        isNewlyPlaced = true;
                        break;
                    }
                }

                if (isNewlyPlaced) {
                    SpecialSquare special = board.getSpecialSquare(row, col);
                    
                    switch (special) {
                        case SpecialSquare::DOUBLE_LETTER:
                            letterPoints *= 2;
                            break;
                        case SpecialSquare::TRIPLE_LETTER:
                            letterPoints *= 3;
                            break;
                        case SpecialSquare::DOUBLE_WORD:
                        case SpecialSquare::CENTER:
                            wordMultiplier *= 2;
                            break;
                        case SpecialSquare::TRIPLE_WORD:
                            wordMultiplier *= 3;
                            break;
                        default:
                            break;
                    }
                }
                
                totalScore += letterPoints;
            }
        }
    }
    
    totalScore *= wordMultiplier;
    
    if (!completeWord.empty()) {
        const float infoX = 20.0f;
        const float infoY = 20.0f;
        
        renderText("Building: " + completeWord, infoX, infoY, BLACK_COLOR, font);
        
        std::string scoreText = "Preview Score: " + std::to_string(totalScore);
        if (wordMultiplier > 1) {
            scoreText += " (x" + std::to_string(wordMultiplier) + " word bonus)";
        }
        
        renderText(scoreText, infoX, infoY + 25.0f, BLUE_COLOR, smallFont);
    }
}

void GameRenderer::renderScores(const Player& player1, const Player& player2) {
}

// === SCREEN RENDERING METHODS ===
void GameRenderer::renderGameStart() {
    static Uint64 startTime = 0;
    if (startTime == 0) {
        startTime = SDL_GetTicks();
    }
    
    Uint64 currentTime = SDL_GetTicks();
    float elapsedTime = (currentTime - startTime) / 1000.0f;
    
    // Base gradient background
    SDL_SetRenderDrawColor(renderer, 30, 60, 120, 255);
    SDL_RenderClear(renderer);
    
    // === CENTERED LAYOUT CONSTANTS ===
    const float MARGIN_FROM_TOP = 30.0f;
    const float TITLE_HEIGHT = 150.0f;
    const float BOX_WIDTH = 500.0f;
    const float BUTTON_WIDTH = 180.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 40.0f;
    
    // === CALCULATE TOTAL CONTENT WIDTH ===
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    
    // Calculate centered positions
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float titleY = MARGIN_FROM_TOP;
    const float contentStartY = titleY + TITLE_HEIGHT + 50.0f;
    
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float boxX = contentGroupStartX;
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;

    renderFullWidthTitle(elapsedTime, titleY, TITLE_HEIGHT);
    renderInformationBoxes(elapsedTime, boxX, contentStartY, BOX_WIDTH);
    renderSideButtons(elapsedTime, buttonX, contentStartY, BUTTON_WIDTH);
}

void GameRenderer::renderMenu() {
    renderMenuBackground();
    renderMenuContent();
}

void GameRenderer::renderPauseButton() {
    const float buttonX = WINDOW_WIDTH - PAUSE_BUTTON_SIZE - PAUSE_BUTTON_MARGIN;
    const float buttonY = PAUSE_BUTTON_MARGIN;
    
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 220);
    const SDL_FRect buttonRect = {buttonX, buttonY, PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE};
    SDL_RenderFillRect(renderer, &buttonRect);
    
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            buttonX - i, buttonY - i,
            PAUSE_BUTTON_SIZE + 2*i, PAUSE_BUTTON_SIZE + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    SDL_SetRenderDrawColor(renderer, WHITE_COLOR.r, WHITE_COLOR.g, WHITE_COLOR.b, WHITE_COLOR.a);
    const SDL_FRect bar1 = {buttonX + 10, buttonY + 6, 8, 28};
    const SDL_FRect bar2 = {buttonX + 22, buttonY + 6, 8, 28};
    SDL_RenderFillRect(renderer, &bar1);
    SDL_RenderFillRect(renderer, &bar2);
    
    // Add "PAUSE" label below button
    renderText("PAUSE", buttonX - 5, buttonY + PAUSE_BUTTON_SIZE + 5, BLACK_COLOR, smallFont);
}

void GameRenderer::renderPauseMenu() {
    // Animation timing - similar to other screens
    static Uint64 startTime = 0;
    if (startTime == 0) {
        startTime = SDL_GetTicks();
    }
    
    Uint64 currentTime = SDL_GetTicks();
    float elapsedTime = (currentTime - startTime) / 1000.0f;
    
    // Dark overlay background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    const SDL_FRect overlay = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_RenderFillRect(renderer, &overlay);
    
    // === LAYOUT CONSTANTS ===
    const float TITLE_HEIGHT = 70.0f;
    const float BOX_WIDTH = 500.0f;
    const float BOX_HEIGHT = 80.0f;
    const float BUTTON_WIDTH = 180.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 30.0f;
    const float GAP_TITLE_TO_CONTENT = 20.0f;
    const float GAP_CONTENT_TO_BUTTONS = 30.0f;
    
    // Calculate centered positions
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float centerY = WINDOW_HEIGHT / 2.0f;
    const float titleY = centerY - 200.0f;
    const float contentStartY = titleY + TITLE_HEIGHT + GAP_TITLE_TO_CONTENT;
    const float buttonStartY = contentStartY + BOX_HEIGHT + GAP_CONTENT_TO_BUTTONS;
    const float boxX = centerX - BOX_WIDTH / 2.0f;
    const float buttonX = centerX - BUTTON_WIDTH / 2.0f;

    renderPauseTitle(elapsedTime, titleY, TITLE_HEIGHT);
    renderPauseContent(elapsedTime, boxX, contentStartY, BOX_WIDTH);
    renderPauseButtons(elapsedTime, buttonX, buttonStartY, BUTTON_WIDTH);
}

bool GameRenderer::isPointInPauseButton(int x, int y) const {
    const float buttonX = WINDOW_WIDTH - PAUSE_BUTTON_SIZE - PAUSE_BUTTON_MARGIN;
    const float buttonY = PAUSE_BUTTON_MARGIN;
    
    return x >= static_cast<int>(buttonX) && x <= static_cast<int>(buttonX + PAUSE_BUTTON_SIZE) &&
           y >= static_cast<int>(buttonY) && y <= static_cast<int>(buttonY + PAUSE_BUTTON_SIZE);
}

PauseMenuOption GameRenderer::getPauseMenuOption(int x, int y) const {
    const float BUTTON_WIDTH = 180.0f;
    const float buttonHeight = 55.0f;
    const float buttonGap = 20.0f;
    
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float centerY = WINDOW_HEIGHT / 2.0f;
    const float titleY = centerY - 200.0f;
    const float TITLE_HEIGHT = 70.0f;
    const float BOX_HEIGHT = 80.0f;
    const float GAP_TITLE_TO_CONTENT = 20.0f;
    const float GAP_CONTENT_TO_BUTTONS = 30.0f;
    
    const float contentStartY = titleY + TITLE_HEIGHT + GAP_TITLE_TO_CONTENT;
    const float buttonStartY = contentStartY + BOX_HEIGHT + GAP_CONTENT_TO_BUTTONS;
    const float buttonX = centerX - BUTTON_WIDTH / 2.0f;
    
    // Check each button area
    for (int i = 0; i < 4; i++) {
        const float buttonY = buttonStartY + i * (buttonHeight + buttonGap);
        
        if (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight) {
            
            switch (i) {
                case 0: return PauseMenuOption::CONTINUE;
                case 1: return PauseMenuOption::SURRENDER;
                case 2: return PauseMenuOption::NEW_GAME;
                case 3: return PauseMenuOption::QUIT;
            }
        }
    }
    
    return static_cast<PauseMenuOption>(-1); // outside buttons
}

void GameRenderer::renderGameOver(const Player& player1, const Player& player2) {
    // Animation timing
    static Uint64 startTime = 0;
    if (startTime == 0) {
        startTime = SDL_GetTicks();
    }
    
    Uint64 currentTime = SDL_GetTicks();
    float elapsedTime = (currentTime - startTime) / 1000.0f;
    
    // Base gradient background
    SDL_SetRenderDrawColor(renderer, 20, 30, 60, 255);
    SDL_RenderClear(renderer);
    
    // === CENTERED LAYOUT CONSTANTS ===
    const float MARGIN_FROM_TOP = 40.0f;
    const float TITLE_HEIGHT = 120.0f;
    const float BOX_WIDTH = 600.0f;
    const float BUTTON_WIDTH = 200.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 50.0f;
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    
    // Calculate centered positions
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float titleY = MARGIN_FROM_TOP;
    const float contentStartY = titleY + TITLE_HEIGHT + 40.0f;
    
    // Center the entire content group
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float boxX = contentGroupStartX;
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    renderGameOverTitle(elapsedTime, titleY, TITLE_HEIGHT);
    renderGameOverContent(elapsedTime, boxX, contentStartY, BOX_WIDTH, player1, player2);
    renderGameOverButtons(elapsedTime, buttonX, contentStartY, BUTTON_WIDTH);
}

void GameRenderer::renderPauseScreen() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    const SDL_FRect overlay = {0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)};
    SDL_RenderFillRect(renderer, &overlay);
    
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
    
    const SDL_Color color = SPECIAL_SQUARE_COLORS[static_cast<int>(special)];
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    const SDL_FRect rect = {x + 1.0f, y + 1.0f, CELL_SIZE - 2.0f, CELL_SIZE - 2.0f};
    SDL_RenderFillRect(renderer, &rect);
    
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
    
    SDL_SetRenderDrawColor(renderer, TILE_COLOR.r, TILE_COLOR.g, TILE_COLOR.b, TILE_COLOR.a);
    SDL_RenderFillRect(renderer, &tileRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &tileRect);
    
    const float tileX = tileRect.x;
    const float tileY = tileRect.y;
    const float tileWidth = tileRect.w;
    const float tileHeight = tileRect.h;

    const std::string letter(1, tile->getLetter());
    
    int textW, textH;
    if (font && TTF_GetStringSize(font, letter.c_str(), 0, &textW, &textH) == 0) {
        float letterX = tileX + 4.0f;
        float letterY = tileY + 2.0f;
        renderText(letter, letterX, letterY, BLACK_COLOR, font);
    } else {
        float letterX = tileX + 4.0f;
        float letterY = tileY + 2.0f;
        renderText(letter, letterX, letterY, BLACK_COLOR, font);
    }

    const std::string points = std::to_string(tile->getPoints());
    
    int pointsW, pointsH;
    if (smallFont && TTF_GetStringSize(smallFont, points.c_str(), 0, &pointsW, &pointsH) == 0) {
        float pointsX = x + CELL_SIZE - pointsW - 3.0f;
        float pointsY = y + CELL_SIZE - pointsH - 10.0f;
        renderText(points, pointsX, pointsY, BLACK_COLOR, smallFont);
    } else {
        float pointsX = x + CELL_SIZE - 12.0f;
        float pointsY = y + CELL_SIZE - 20.0f;
        renderText(points, pointsX, pointsY, BLACK_COLOR, smallFont);
    }
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
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const SDL_FRect clearRect = {
        x - 5.0f, y - 5.0f,
        7.0f * TILE_SPACING + 10.0f,
        50.0f
    };
    SDL_RenderFillRect(renderer, &clearRect);
    
    const float totalWidth = 7.0f * TILE_SPACING;
    const float actualRackWidth = static_cast<float>(rack.size()) * TILE_SPACING;
    const float centerOffset = (totalWidth - actualRackWidth) / 2.0f;
    
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
    specialFont = TTF_OpenFont(path.c_str(), SPECIAL_FONT_SIZE);
    
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
    if (specialFont) { TTF_CloseFont(specialFont); specialFont = nullptr; }
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

void GameRenderer::renderFullWidthTitle(float elapsedTime, float startY, float titleHeight) {
    const float TITLE_ANIMATION_DURATION = 3.0f;
    const float centerX = WINDOW_WIDTH / 2.0f;
    
    if (elapsedTime < TITLE_ANIMATION_DURATION) {
        // Animated title entrance
        float animProgress = elapsedTime / TITLE_ANIMATION_DURATION;
        float bounceHeight = 20.0f * (1.0f - animProgress) * sin(animProgress * 6.0f);
        float sizeMultiplier = 2.0f - (1.0f * animProgress);
        float currentY = startY + (titleHeight / 3.0f) + bounceHeight;
        
        // Color animation with glow pulse
        float glowPulse = 1.0f + 0.3f * sin(elapsedTime * 4.0f);
        SDL_Color mainColor = {
            static_cast<Uint8>(std::min(255.0f, 220 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 60 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 60 * glowPulse)),
            255
        };
        
        // Bloom glow effect
        for (int glow = 6; glow >= 1; glow--) {
            SDL_Color glowColor = {
                static_cast<Uint8>(mainColor.r / 2),
                static_cast<Uint8>(mainColor.g / 2),
                static_cast<Uint8>(mainColor.b / 2),
                static_cast<Uint8>(40 / glow)
            };
            
            float glowOffset = glow * 2.0f * glowPulse;
            renderText("SCRABBLE GAME", centerX - 200 + glowOffset, currentY + glowOffset, glowColor, specialFont);
        }
        
        // 3D shadow effect
        renderText("SCRABBLE GAME", centerX - 200 + 4, currentY + 4, {80, 80, 80, 255}, specialFont);
        renderText("SCRABBLE GAME", centerX - 200 + 2, currentY + 2, {120, 120, 120, 255}, specialFont);
        
        // Main title text
        renderText("SCRABBLE GAME", centerX - 200, currentY, mainColor, specialFont);
        
    } else {
        // Static title with subtle pulse
        float staticGlow = 1.0f + 0.15f * sin(elapsedTime * 3.0f);
        float finalY = startY + (titleHeight / 3.0f);
        
        // Bloom glow for static title
        for (int glow = 4; glow >= 1; glow--) {
            SDL_Color glowColor = {
                static_cast<Uint8>(RED_COLOR.r / 3),
                static_cast<Uint8>(RED_COLOR.g / 4),
                static_cast<Uint8>(RED_COLOR.b / 4),
                static_cast<Uint8>(20 / glow)
            };
            
            float glowOffset = glow * 1.5f * staticGlow;
            renderText("SCRABBLE GAME", centerX - 200 + glowOffset, finalY + glowOffset, glowColor, specialFont);
        }
        
        // 3D static effect
        renderText("SCRABBLE GAME", centerX - 200 + 3, finalY + 3, {80, 80, 80, 255}, specialFont);
        renderText("SCRABBLE GAME", centerX - 200 + 1, finalY + 1, {120, 120, 120, 255}, specialFont);
        
        // Main text with pulse
        SDL_Color pulsedRed = {
            static_cast<Uint8>(RED_COLOR.r * staticGlow),
            static_cast<Uint8>(RED_COLOR.g * staticGlow),
            static_cast<Uint8>(RED_COLOR.b * staticGlow),
            255
        };
        renderText("SCRABBLE GAME", centerX - 200, finalY, pulsedRed, specialFont);
    }
}

void GameRenderer::renderInformationBoxes(float elapsedTime, float startX, float startY, float boxWidth) {
    const float CONTENT_DELAY = 2.0f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    
    float contentProgress = (elapsedTime - CONTENT_DELAY) / 1.5f;
    if (contentProgress > 1.0f) contentProgress = 1.0f;
    
    const float boxHeight1 = 260.0f;
    const float boxHeight2 = 230.0f;
    const float gapBetweenBoxes = 25.0f;
    
    // Slide in animation
    float slideOffset = (1.0f - contentProgress) * 50.0f;
    float currentBoxX = startX - slideOffset;
    
    // === GAME INFORMATION BOX ===
    const SDL_FRect gameInfoBox = {currentBoxX, startY, boxWidth, boxHeight1};
    
    // Shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(60 * contentProgress));
    const SDL_FRect shadowRect1 = {gameInfoBox.x + 5, gameInfoBox.y + 5, boxWidth, boxHeight1};
    SDL_RenderFillRect(renderer, &shadowRect1);
    
    // Main box
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, static_cast<Uint8>(240 * contentProgress));
    SDL_RenderFillRect(renderer, &gameInfoBox);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, static_cast<Uint8>(200 * contentProgress));
    SDL_RenderRect(renderer, &gameInfoBox);
    
    // CENTERED CONTENT with better spacing
    if (contentProgress > 0.7f) {
        float textAlpha = (contentProgress - 0.7f) / 0.3f;
        SDL_Color fadeColor = {0, 0, 0, static_cast<Uint8>(255 * textAlpha)};
        SDL_Color blueColor = {59, 130, 246, static_cast<Uint8>(255 * textAlpha)};
        
        float contentY = gameInfoBox.y + 20.0f;
        
        // CENTERED TITLE
        const std::string titleText = "Welcome to Scrabble!";
        int titleW, titleH;
        if (titleFont && TTF_GetStringSize(titleFont, titleText.c_str(), 0, &titleW, &titleH) == 0) {
            float titleX = gameInfoBox.x + (boxWidth - titleW) / 2.0f;
            renderText(titleText, titleX, contentY, blueColor, titleFont);
        } else {
            renderText(titleText, gameInfoBox.x + 20, contentY, blueColor, titleFont);
        }
        contentY += 40.0f; // Reduced spacing
        
        const std::vector<std::string> gameInfo = {
            "• Create words from letter tiles",
            "• Challenge your vocabulary skills", 
            "• Compete against friends",
            "• Every game is unique and exciting!",
            "• Score higher by using premium squares",
            "• Build off existing words for more points"
        };
        
        for (const auto& info : gameInfo) {
            renderText(info, gameInfoBox.x + 25, contentY, fadeColor, font);
            contentY += 28.0f;
        }
    }
    
    // === ANIMATED TUTORIAL BOX ===
    // Calculate animation progress
    float tutorialHeight = 0.0f;
    float tutorialAlpha = 0.0f;
    
    if (tutorialAnimating) {
        Uint64 currentTime = SDL_GetTicks();
        float animTime = (currentTime - tutorialAnimationStart) / 1000.0f;
        float animProgress = std::min(1.0f, animTime / TUTORIAL_ANIMATION_DURATION);
        
        if (tutorialVisible) {
            tutorialHeight = boxHeight2 * animProgress;
            tutorialAlpha = animProgress;
        } else {
            tutorialHeight = boxHeight2 * (1.0f - animProgress);
            tutorialAlpha = 1.0f - animProgress;
        }
        
        if (animProgress >= 1.0f) {
            tutorialAnimating = false;
        }
    } else if (tutorialVisible) {
        tutorialHeight = boxHeight2;
        tutorialAlpha = 1.0f;
    }
    
    // Render tutorial box
    if (tutorialHeight > 10.0f && contentProgress > 0.5f) {
        float tutorialY = startY + boxHeight1 + gapBetweenBoxes;
        const SDL_FRect tutorialBox = {currentBoxX, tutorialY, boxWidth, tutorialHeight};
        
        // Shadow
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(60 * contentProgress * tutorialAlpha));
        const SDL_FRect shadowRect2 = {tutorialBox.x + 5, tutorialBox.y + 5, boxWidth, tutorialHeight};
        SDL_RenderFillRect(renderer, &shadowRect2);
        
        // Main box
        SDL_SetRenderDrawColor(renderer, 250, 250, 250, static_cast<Uint8>(240 * contentProgress * tutorialAlpha));
        SDL_RenderFillRect(renderer, &tutorialBox);
        
        // Animated border with glow
        float borderGlow = 1.0f + 0.1f * sin(elapsedTime * 2.0f);
        SDL_SetRenderDrawColor(renderer, 
            static_cast<Uint8>(100 * borderGlow), 
            static_cast<Uint8>(149 * borderGlow), 
            static_cast<Uint8>(237 * borderGlow), 
            static_cast<Uint8>(200 * contentProgress * tutorialAlpha));
        SDL_RenderRect(renderer, &tutorialBox);
        
        // Tutorial content - only show if box is reasonably open
        if (tutorialHeight > boxHeight2 * 0.3f && contentProgress > 0.8f) {
            float textAlpha = (contentProgress - 0.8f) / 0.2f * tutorialAlpha;
            SDL_Color fadeColor = {0, 0, 0, static_cast<Uint8>(255 * textAlpha)};
            SDL_Color blueColor = {59, 130, 246, static_cast<Uint8>(255 * textAlpha)};
            
            float tutorialContentY = tutorialBox.y + 15.0f;
            
            // CENTERED TUTORIAL TITLE
            const std::string tutorialTitle = "How to Play:";
            int tutTitleW, tutTitleH;
            if (titleFont && TTF_GetStringSize(titleFont, tutorialTitle.c_str(), 0, &tutTitleW, &tutTitleH) == 0) {
                float tutTitleX = tutorialBox.x + (boxWidth - tutTitleW) / 2.0f;
                renderText(tutorialTitle, tutTitleX, tutorialContentY, blueColor, titleFont);
            } else {
                renderText(tutorialTitle, tutorialBox.x + 20, tutorialContentY, blueColor, titleFont);
            }
            tutorialContentY += 35.0f; // Reduced spacing
            
            const std::vector<std::string> instructions = {
                "• Click tiles to select, click board to place",
                "• Press 1-7 to select rack tiles directly",
                "• ENTER to confirm word placement",
                "• BACKSPACE to cancel current word",
                "• S to shuffle your tile rack",
                "• SPACE to skip your turn",
                "• ESC to pause or quit game"
            };
            
            for (const auto& instruction : instructions) {
                if (tutorialContentY + 25.0f < tutorialBox.y + tutorialHeight - 10.0f) {
                    renderText(instruction, tutorialBox.x + 25, tutorialContentY, fadeColor, font);
                    tutorialContentY += 25.0f;
                }
            }
        }
    }
}  

void GameRenderer::renderSideButtons(float elapsedTime, float startX, float startY, float buttonWidth) {
    const float CONTENT_DELAY = 2.0f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    float buttonProgress = (elapsedTime - CONTENT_DELAY) / 1.0f;
    if (buttonProgress > 1.0f) buttonProgress = 1.0f;
    
    const float buttonHeight = 65.0f;
    const float buttonGap = 20.0f;
    
    // Slide in from right
    float slideOffset = (1.0f - buttonProgress) * 100.0f;
    float currentButtonX = startX + slideOffset;
    
    // Button definitions
    struct ButtonInfo {
        std::string text;
        std::string subtitle;
        SDL_Color primaryColor;
        SDL_Color accentColor;
        float yOffset;
        bool isActive;
    };
    
    std::vector<ButtonInfo> buttons = {
        {"START PLAYING", "Begin Your Adventure", {34, 197, 94, 255}, {22, 163, 74, 255}, 0.0f, true},
        {"HOW TO PLAY", "Learn the Rules", {59, 130, 246, 255}, {37, 99, 235, 255}, buttonHeight + buttonGap, true},
        {"EXIT GAME", "Quit Application", {239, 68, 68, 255}, {220, 38, 127, 255}, 2 * (buttonHeight + buttonGap), true}
    };
    
    buttons[1].text = tutorialVisible ? "HIDE TUTORIAL" : "HOW TO PLAY";
    buttons[1].subtitle = tutorialVisible ? "Close Guide" : "Learn the Rules";
    
    float pulseEffect = 1.0f + 0.03f * sin(elapsedTime * 3.0f);
    
    for (const auto& btn : buttons) {
        if (!btn.isActive) continue;
        
        const SDL_FRect buttonRect = {
            currentButtonX, startY + btn.yOffset,
            buttonWidth, buttonHeight
        };
        
        // Shadow
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(40 * buttonProgress));
        const SDL_FRect shadowRect = {
            buttonRect.x + 3, buttonRect.y + 3, 
            buttonRect.w * pulseEffect, buttonRect.h * pulseEffect
        };
        SDL_RenderFillRect(renderer, &shadowRect);
        
        // Main button
        SDL_SetRenderDrawColor(renderer, 
            btn.primaryColor.r, btn.primaryColor.g, btn.primaryColor.b, 
            static_cast<Uint8>(240 * buttonProgress));
        const SDL_FRect pulsedRect = {
            buttonRect.x, buttonRect.y,
            buttonRect.w * pulseEffect, buttonRect.h * pulseEffect
        };
        SDL_RenderFillRect(renderer, &pulsedRect);
        
        // Border
        SDL_SetRenderDrawColor(renderer, 
            btn.accentColor.r, btn.accentColor.g, btn.accentColor.b, 
            static_cast<Uint8>(255 * buttonProgress));
        SDL_RenderRect(renderer, &pulsedRect);
        
        // Centered button text
        SDL_Color mainTextColor = {255, 255, 255, static_cast<Uint8>(255 * buttonProgress)};
        SDL_Color subTextColor = {255, 255, 255, static_cast<Uint8>(200 * buttonProgress)};
        
        // Center main text
        int mainTextW, mainTextH;
        if (titleFont && TTF_GetStringSize(titleFont, btn.text.c_str(), 0, &mainTextW, &mainTextH) == 0) {
            float mainTextX = buttonRect.x + (buttonWidth - mainTextW) / 2.0f;
            renderText(btn.text, mainTextX, buttonRect.y + 12, mainTextColor, titleFont);
        } else {
            renderText(btn.text, buttonRect.x + 15, buttonRect.y + 12, mainTextColor, titleFont);
        }
        
        // Center subtitle
        int subTextW, subTextH;
        if (font && TTF_GetStringSize(font, btn.subtitle.c_str(), 0, &subTextW, &subTextH) == 0) {
            float subTextX = buttonRect.x + (buttonWidth - subTextW) / 2.0f;
            renderText(btn.subtitle, subTextX, buttonRect.y + 40, subTextColor, font);
        } else {
            renderText(btn.subtitle, buttonRect.x + 15, buttonRect.y + 40, subTextColor, font);
        }
    }
}

void GameRenderer::renderGameOverTitle(float elapsedTime, float startY, float titleHeight) {
    const float TITLE_ANIMATION_DURATION = 2.5f;
    const float centerX = WINDOW_WIDTH / 2.0f;
    
    if (elapsedTime < TITLE_ANIMATION_DURATION) {
        // Dramatic title entrance with bounce
        float animProgress = elapsedTime / TITLE_ANIMATION_DURATION;
        float bounceHeight = 30.0f * (1.0f - animProgress) * sin(animProgress * 8.0f);
        float currentY = startY + (titleHeight / 4.0f) + bounceHeight;
        
        // Color animation - more dramatic for game over
        float glowPulse = 1.0f + 0.5f * sin(elapsedTime * 5.0f);
        SDL_Color mainColor = {
            static_cast<Uint8>(std::min(255.0f, 255 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 50 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 50 * glowPulse)),
            255
        };
        
        // Calculate centered X position for "GAME OVER"
        int titleTextWidth, titleTextHeight;
        if (specialFont && TTF_GetStringSize(specialFont, "GAME OVER", 0, &titleTextWidth, &titleTextHeight) == 0) {
            float centeredTitleX = centerX - (titleTextWidth / 2.0f);
            
            // Massive bloom glow effect
            for (int glow = 12; glow >= 1; glow--) {
                SDL_Color glowColor = {
                    static_cast<Uint8>(mainColor.r / 2),
                    static_cast<Uint8>(mainColor.g / 3),
                    static_cast<Uint8>(mainColor.b / 3),
                    static_cast<Uint8>(50 / glow)
                };
                
                float glowOffset = glow * 4.0f * glowPulse;
                renderText("GAME OVER", centeredTitleX + glowOffset, currentY + glowOffset, glowColor, specialFont);
            }
            
            // Multiple shadow layers for depth
            renderText("GAME OVER", centeredTitleX + 8, currentY + 8, {40, 40, 40, 255}, specialFont);
            renderText("GAME OVER", centeredTitleX + 6, currentY + 6, {60, 60, 60, 255}, specialFont);
            renderText("GAME OVER", centeredTitleX + 4, currentY + 4, {80, 80, 80, 255}, specialFont);
            renderText("GAME OVER", centeredTitleX + 2, currentY + 2, {120, 120, 120, 255}, specialFont);
            
            // Main title text - CENTERED
            renderText("GAME OVER", centeredTitleX, currentY, mainColor, specialFont);
        } else {
            float fallbackX = centerX - 120.0f;
            
            for (int glow = 12; glow >= 1; glow--) {
                SDL_Color glowColor = {
                    static_cast<Uint8>(mainColor.r / 2),
                    static_cast<Uint8>(mainColor.g / 3),
                    static_cast<Uint8>(mainColor.b / 3),
                    static_cast<Uint8>(50 / glow)
                };
                
                float glowOffset = glow * 4.0f * glowPulse;
                renderText("GAME OVER", fallbackX + glowOffset, currentY + glowOffset, glowColor, specialFont);
            }
            
            // Shadow layers
            renderText("GAME OVER", fallbackX + 8, currentY + 8, {40, 40, 40, 255}, specialFont);
            renderText("GAME OVER", fallbackX + 6, currentY + 6, {60, 60, 60, 255}, specialFont);
            renderText("GAME OVER", fallbackX + 4, currentY + 4, {80, 80, 80, 255}, specialFont);
            renderText("GAME OVER", fallbackX + 2, currentY + 2, {120, 120, 120, 255}, specialFont);
            
            // Main title text
            renderText("GAME OVER", fallbackX, currentY, mainColor, specialFont);
        }
        
    } else {
        // Static title with dramatic pulse
        float staticGlow = 1.0f + 0.2f * sin(elapsedTime * 2.5f);
        float finalY = startY + (titleHeight / 4.0f);
        
        int titleTextWidth, titleTextHeight;
        if (specialFont && TTF_GetStringSize(specialFont, "GAME OVER", 0, &titleTextWidth, &titleTextHeight) == 0) {
            float centeredTitleX = centerX - (titleTextWidth / 2.0f);
            
            // Dramatic static glow
            for (int glow = 8; glow >= 1; glow--) {
                SDL_Color glowColor = {
                    static_cast<Uint8>(RED_COLOR.r / 2),
                    static_cast<Uint8>(RED_COLOR.g / 4),
                    static_cast<Uint8>(RED_COLOR.b / 4),
                    static_cast<Uint8>(30 / glow)
                };
                
                float glowOffset = glow * 3.0f * staticGlow;
                renderText("GAME OVER", centeredTitleX + glowOffset, finalY + glowOffset, glowColor, specialFont);
            }
            
            // 3D depth effect
            renderText("GAME OVER", centeredTitleX + 6, finalY + 6, {60, 60, 60, 255}, specialFont);
            renderText("GAME OVER", centeredTitleX + 3, finalY + 3, {100, 100, 100, 255}, specialFont);
            renderText("GAME OVER", centeredTitleX + 1, finalY + 1, {140, 140, 140, 255}, specialFont);
            
            SDL_Color pulsedRed = {
                static_cast<Uint8>(RED_COLOR.r * staticGlow),
                static_cast<Uint8>(RED_COLOR.g * staticGlow),
                static_cast<Uint8>(RED_COLOR.b * staticGlow),
                255
            };
            renderText("GAME OVER", centeredTitleX, finalY, pulsedRed, specialFont);
        } else {
            float fallbackX = centerX - 120.0f;
            
            // Static glow effects
            for (int glow = 8; glow >= 1; glow--) {
                SDL_Color glowColor = {
                    static_cast<Uint8>(RED_COLOR.r / 2),
                    static_cast<Uint8>(RED_COLOR.g / 4),
                    static_cast<Uint8>(RED_COLOR.b / 4),
                    static_cast<Uint8>(30 / glow)
                };
                
                float glowOffset = glow * 3.0f * staticGlow;
                renderText("GAME OVER", fallbackX + glowOffset, finalY + glowOffset, glowColor, specialFont);
            }
            
            // Shadow effects
            renderText("GAME OVER", fallbackX + 6, finalY + 6, {60, 60, 60, 255}, specialFont);
            renderText("GAME OVER", fallbackX + 3, finalY + 3, {100, 100, 100, 255}, specialFont);
            renderText("GAME OVER", fallbackX + 1, finalY + 1, {140, 140, 140, 255}, specialFont);
            
            // Main text
            SDL_Color pulsedRed = {
                static_cast<Uint8>(RED_COLOR.r * staticGlow),
                static_cast<Uint8>(RED_COLOR.g * staticGlow),
                static_cast<Uint8>(RED_COLOR.b * staticGlow),
                255
            };
            renderText("GAME OVER", fallbackX, finalY, pulsedRed, specialFont);
        }
    }
}

void GameRenderer::renderGameOverContent(float elapsedTime, float startX, float startY, float boxWidth, const Player& player1, const Player& player2) {
    const float CONTENT_DELAY = 1.5f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    
    float contentProgress = (elapsedTime - CONTENT_DELAY) / 1.5f;
    if (contentProgress > 1.0f) contentProgress = 1.0f;
    
    const float winnerBoxHeight = 150.0f;
    const float scoresBoxHeight = 300.0f;
    const float gapBetweenBoxes = 30.0f;
    
    // Slide in animation
    float slideOffset = (1.0f - contentProgress) * 150.0f;
    float currentBoxX = startX - slideOffset;
    
    // Determine winner
    std::string winnerText;
    SDL_Color winnerColor;
    bool isTie = (player1.getScore() == player2.getScore());
    
    if (isTie) {
        winnerText = "IT'S A TIE!";
        winnerColor = {255, 215, 0, 255}; // Gold
    } else if (player1.getScore() > player2.getScore()) {
        winnerText = player1.getName() + " WINS!";
        winnerColor = {34, 197, 94, 255}; // Green
    } else {
        winnerText = player2.getName() + " WINS!";
        winnerColor = {34, 197, 94, 255}; // Green
    }
    
    // === WINNER ANNOUNCEMENT BOX ===
    const SDL_FRect winnerBox = {currentBoxX, startY, boxWidth, winnerBoxHeight};
    
    // Shadow with glow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(80 * contentProgress));
    const SDL_FRect shadowRect1 = {winnerBox.x + 6, winnerBox.y + 6, boxWidth, winnerBoxHeight};
    SDL_RenderFillRect(renderer, &shadowRect1);
    
    // Main winner box with gradient effect
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, static_cast<Uint8>(245 * contentProgress));
    SDL_RenderFillRect(renderer, &winnerBox);
    
    // Animated border with victory glow
    float victoryGlow = 1.0f + 0.2f * sin(elapsedTime * 3.0f);
    SDL_Color borderColor = {
        static_cast<Uint8>(winnerColor.r * victoryGlow),
        static_cast<Uint8>(winnerColor.g * victoryGlow),
        static_cast<Uint8>(winnerColor.b * victoryGlow),
        static_cast<Uint8>(255 * contentProgress)
    };
    
    for (int i = 0; i < 3; i++) {
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, static_cast<Uint8>((255 - i * 50) * contentProgress));
        const SDL_FRect borderRect = {
            winnerBox.x - i, winnerBox.y - i,
            winnerBox.w + 2*i, winnerBox.h + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // Winner content
    if (contentProgress > 0.6f) {
        float textAlpha = (contentProgress - 0.6f) / 0.4f;
        SDL_Color fadedWinnerColor = {winnerColor.r, winnerColor.g, winnerColor.b, static_cast<Uint8>(255 * textAlpha)};
        SDL_Color fadedBlackColor = {0, 0, 0, static_cast<Uint8>(255 * textAlpha)};
        
        float winnerContentY = winnerBox.y + 30.0f;
        
        // Victory message
        renderText(winnerText, winnerBox.x + 50, winnerContentY, fadedWinnerColor, titleFont);
        winnerContentY += 50.0f;
        
        // Score difference
        int scoreDiff = abs(player1.getScore() - player2.getScore());
        if (!isTie) {
            std::string marginText = "Victory Margin: " + std::to_string(scoreDiff) + " points";
            renderText(marginText, winnerBox.x + 50, winnerContentY, fadedBlackColor, font);
        } else {
            renderText("Perfect Match!", winnerBox.x + 50, winnerContentY, fadedBlackColor, font);
        }
    }
    
    // === DETAILED SCORES BOX ===
    float scoresY = startY + winnerBoxHeight + gapBetweenBoxes;
    const SDL_FRect scoresBox = {currentBoxX, scoresY, boxWidth, scoresBoxHeight};
    
    // Shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(60 * contentProgress));
    const SDL_FRect shadowRect2 = {scoresBox.x + 5, scoresBox.y + 5, boxWidth, scoresBoxHeight};
    SDL_RenderFillRect(renderer, &shadowRect2);
    
    // Main scores box
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, static_cast<Uint8>(240 * contentProgress));
    SDL_RenderFillRect(renderer, &scoresBox);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, static_cast<Uint8>(200 * contentProgress));
    SDL_RenderRect(renderer, &scoresBox);
    
    // Scores content
    if (contentProgress > 0.8f) {
        float textAlpha = (contentProgress - 0.8f) / 0.2f;
        SDL_Color fadeColor = {0, 0, 0, static_cast<Uint8>(255 * textAlpha)};
        SDL_Color blueColor = {59, 130, 246, static_cast<Uint8>(255 * textAlpha)};
        SDL_Color greenColor = {34, 197, 94, static_cast<Uint8>(255 * textAlpha)};
        
        float scoresContentY = scoresBox.y + 25.0f;
        
        // Title
        renderText("Final Scores", scoresBox.x + 25, scoresContentY, blueColor, titleFont);
        scoresContentY += 50.0f;
        
        // Player 1 score
        SDL_Color player1Color = (player1.getScore() >= player2.getScore() && !isTie) ? greenColor : fadeColor;
        std::string score1Text = player1.getName() + ": " + std::to_string(player1.getScore()) + " points";
        renderText(score1Text, scoresBox.x + 30, scoresContentY, player1Color, font);
        scoresContentY += 35.0f;
        
        // Player 2 score  
        SDL_Color player2Color = (player2.getScore() >= player1.getScore() && !isTie) ? greenColor : fadeColor;
        std::string score2Text = player2.getName() + ": " + std::to_string(player2.getScore()) + " points";
        renderText(score2Text, scoresBox.x + 30, scoresContentY, player2Color, font);
        scoresContentY += 50.0f;
        
        // Game statistics
        renderText("Game Statistics:", scoresBox.x + 25, scoresContentY, blueColor, font);
        scoresContentY += 30.0f;
        
        const std::vector<std::string> stats = {
            "• Total tiles used: " + std::to_string(14 - player1.getRackSize() - player2.getRackSize()),
            "• " + player1.getName() + " tiles remaining: " + std::to_string(player1.getRackSize()),
            "• " + player2.getName() + " tiles remaining: " + std::to_string(player2.getRackSize()),
            "• Thanks for playing!"
        };
        
        for (const auto& stat : stats) {
            renderText(stat, scoresBox.x + 30, scoresContentY, fadeColor, smallFont);
            scoresContentY += 25.0f;
        }
    }
}

void GameRenderer::renderGameOverButtons(float elapsedTime, float startX, float startY, float buttonWidth) {
    const float CONTENT_DELAY = 2.0f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    
    float buttonProgress = (elapsedTime - CONTENT_DELAY) / 1.0f;
    if (buttonProgress > 1.0f) buttonProgress = 1.0f;
    
    const float buttonHeight = 70.0f;
    const float buttonGap = 25.0f;
    
    // Slide in from right
    float slideOffset = (1.0f - buttonProgress) * 200.0f;
    float currentButtonX = startX + slideOffset;
    
    // Button definitions - REMOVED VIEW STATS BUTTON
    struct ButtonInfo {
        std::string text;
        std::string subtitle;
        SDL_Color primaryColor;
        SDL_Color accentColor;
        float yOffset;
        bool isActive;
    };
    
    std::vector<ButtonInfo> buttons = {
        {"PLAY AGAIN", "Start New Game", {34, 197, 94, 255}, {22, 163, 74, 255}, 0.0f, true},
        {"MAIN MENU", "Return to Start", {156, 163, 175, 255}, {107, 114, 128, 255}, buttonHeight + buttonGap, true},
        {"EXIT GAME", "Quit Application", {239, 68, 68, 255}, {220, 38, 127, 255}, 2 * (buttonHeight + buttonGap), true}
    };
    
    // Victory pulse effect
    float victoryPulse = 1.0f + 0.08f * sin(elapsedTime * 4.0f);
    
    for (const auto& btn : buttons) {
        if (!btn.isActive) continue;
        
        const SDL_FRect buttonRect = {
            currentButtonX, startY + btn.yOffset,
            buttonWidth, buttonHeight
        };
        
        // Shadow with depth
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(60 * buttonProgress));
        const SDL_FRect shadowRect = {
            buttonRect.x + 4, buttonRect.y + 4, 
            buttonRect.w * victoryPulse, buttonRect.h * victoryPulse
        };
        SDL_RenderFillRect(renderer, &shadowRect);
        
        // Main button with victory glow
        SDL_SetRenderDrawColor(renderer, 
            btn.primaryColor.r, btn.primaryColor.g, btn.primaryColor.b, 
            static_cast<Uint8>(250 * buttonProgress));
        const SDL_FRect pulsedRect = {
            buttonRect.x, buttonRect.y,
            buttonRect.w * victoryPulse, buttonRect.h * victoryPulse
        };
        SDL_RenderFillRect(renderer, &pulsedRect);
        
        // Animated border
        for (int i = 0; i < 2; i++) {
            SDL_SetRenderDrawColor(renderer, 
                btn.accentColor.r, btn.accentColor.g, btn.accentColor.b, 
                static_cast<Uint8>((255 - i * 80) * buttonProgress));
            const SDL_FRect borderRect = {
                pulsedRect.x - i, pulsedRect.y - i,
                pulsedRect.w + 2*i, pulsedRect.h + 2*i
            };
            SDL_RenderRect(renderer, &borderRect);
        }
        
        // Button text
        SDL_Color mainTextColor = {255, 255, 255, static_cast<Uint8>(255 * buttonProgress)};
        SDL_Color subTextColor = {255, 255, 255, static_cast<Uint8>(220 * buttonProgress)};
        
        renderText(btn.text, buttonRect.x + 15, buttonRect.y + 15, mainTextColor, font);
        renderText(btn.subtitle, buttonRect.x + 15, buttonRect.y + 42, subTextColor, smallFont);
    }
}

void GameRenderer::renderPauseTitle(float elapsedTime, float startY, float titleHeight) {
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float TITLE_ANIMATION_DURATION = 1.0f;
    
    if (elapsedTime < TITLE_ANIMATION_DURATION) {
        // Quick bounce entrance
        float animProgress = elapsedTime / TITLE_ANIMATION_DURATION;
        float bounceHeight = 15.0f * (1.0f - animProgress) * sin(animProgress * 4.0f);
        float currentY = startY + bounceHeight;
        
        // Glow effect
        float glowPulse = 1.0f + 0.2f * sin(elapsedTime * 6.0f);
        SDL_Color titleColor = {
            static_cast<Uint8>(std::min(255.0f, 255 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 200 * glowPulse)),
            static_cast<Uint8>(std::min(255.0f, 50 * glowPulse)),
            255
        };
        
        // Shadow effect
        renderText("GAME PAUSED", centerX - 120 + 3, currentY + 3, {80, 80, 80, 255}, specialFont);
        renderText("GAME PAUSED", centerX - 120, currentY, titleColor, specialFont);
    } else {
        // Static with subtle pulse
        float staticGlow = 1.0f + 0.1f * sin(elapsedTime * 3.0f);
        SDL_Color titleColor = {
            static_cast<Uint8>(255 * staticGlow),
            static_cast<Uint8>(200 * staticGlow),
            static_cast<Uint8>(50 * staticGlow),
            255
        };
        
        renderText("GAME PAUSED", centerX - 120 + 2, startY + 2, {100, 100, 100, 255}, specialFont);
        renderText("GAME PAUSED", centerX - 120, startY, titleColor, specialFont);
    }
}

void GameRenderer::renderPauseContent(float elapsedTime, float startX, float startY, float boxWidth) {
    const float CONTENT_DELAY = 0.5f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    
    float contentProgress = (elapsedTime - CONTENT_DELAY) / 0.8f;
    if (contentProgress > 1.0f) contentProgress = 1.0f;
    
    const float boxHeight = 70.0f;
    
    // Slide in from top
    float slideOffset = (1.0f - contentProgress) * 40.0f;
    float currentBoxY = startY - slideOffset;
    
    const SDL_FRect messageBox = {startX, currentBoxY, boxWidth, boxHeight};
    
    // Shadow
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(100 * contentProgress));
    const SDL_FRect shadowRect = {messageBox.x + 4, messageBox.y + 4, boxWidth, boxHeight};
    SDL_RenderFillRect(renderer, &shadowRect);
    
    // Main box
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, static_cast<Uint8>(250 * contentProgress));
    SDL_RenderFillRect(renderer, &messageBox);
    
    // Border with glow
    float borderGlow = 1.0f + 0.15f * sin(elapsedTime * 2.0f);
    SDL_SetRenderDrawColor(renderer, 
        static_cast<Uint8>(100 * borderGlow), 
        static_cast<Uint8>(149 * borderGlow), 
        static_cast<Uint8>(237 * borderGlow), 
        static_cast<Uint8>(200 * contentProgress));
    
    for (int i = 0; i < 2; i++) {
        const SDL_FRect borderRect = {
            messageBox.x - i, messageBox.y - i,
            messageBox.w + 2*i, messageBox.h + 2*i
        };
        SDL_RenderRect(renderer, &borderRect);
    }
    
    // Content text
    if (contentProgress > 0.7f) {
        float textAlpha = (contentProgress - 0.7f) / 0.3f;
        SDL_Color textColor = {0, 0, 0, static_cast<Uint8>(255 * textAlpha)};
        SDL_Color blueColor = {59, 130, 246, static_cast<Uint8>(255 * textAlpha)};
        
        float contentY = messageBox.y + 12.0f;
        renderText("Game is temporarily paused", messageBox.x + 30, contentY, blueColor, font);
        contentY += 25.0f;
        renderText("Choose an option below to continue", messageBox.x + 30, contentY, textColor, smallFont);
    }
}

void GameRenderer::renderPauseButtons(float elapsedTime, float startX, float startY, float buttonWidth) {
    const float CONTENT_DELAY = 1.0f;
    
    if (elapsedTime <= CONTENT_DELAY) return;
    
    float buttonProgress = (elapsedTime - CONTENT_DELAY) / 0.8f;
    if (buttonProgress > 1.0f) buttonProgress = 1.0f;
    
    const float buttonHeight = 55.0f;
    const float buttonGap = 20.0f;
    
    // Slide in from bottom
    float slideOffset = (1.0f - buttonProgress) * 80.0f;
    
    // Button definitions matching your original pause menu options
    struct PauseButtonInfo {
        std::string text;
        std::string subtitle;
        SDL_Color primaryColor;
        SDL_Color accentColor;
        float yOffset;
        PauseMenuOption option;
    };
    
    std::vector<PauseButtonInfo> buttons = {
        {"CONTINUE", "Resume Game", {34, 197, 94, 255}, {22, 163, 74, 255}, 0.0f, PauseMenuOption::CONTINUE},
        {"SURRENDER", "Give Up Turn", {239, 68, 68, 255}, {220, 38, 127, 255}, buttonHeight + buttonGap, PauseMenuOption::SURRENDER},
        {"NEW GAME", "Start Fresh", {59, 130, 246, 255}, {37, 99, 235, 255}, 2 * (buttonHeight + buttonGap), PauseMenuOption::NEW_GAME},
        {"QUIT", "Exit Game", {156, 163, 175, 255}, {107, 114, 128, 255}, 3 * (buttonHeight + buttonGap), PauseMenuOption::QUIT}
    };
    
    float pausePulse = 1.0f + 0.03f * sin(elapsedTime * 4.0f);
    
    for (const auto& btn : buttons) {
        const SDL_FRect buttonRect = {
            startX, startY + btn.yOffset + slideOffset,
            buttonWidth, buttonHeight
        };
        
        // Shadow
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(80 * buttonProgress));
        const SDL_FRect shadowRect = {
            buttonRect.x + 3, buttonRect.y + 3,
            buttonRect.w * pausePulse, buttonRect.h * pausePulse
        };
        SDL_RenderFillRect(renderer, &shadowRect);
        
        // Main button
        SDL_SetRenderDrawColor(renderer, 
            btn.primaryColor.r, btn.primaryColor.g, btn.primaryColor.b, 
            static_cast<Uint8>(230 * buttonProgress));
        const SDL_FRect pulsedRect = {
            buttonRect.x, buttonRect.y,
            buttonRect.w * pausePulse, buttonRect.h * pausePulse
        };
        SDL_RenderFillRect(renderer, &pulsedRect);
        
        // Border
        SDL_SetRenderDrawColor(renderer, 
            btn.accentColor.r, btn.accentColor.g, btn.accentColor.b, 
            static_cast<Uint8>(255 * buttonProgress));
        SDL_RenderRect(renderer, &pulsedRect);
        
        // Button text
        SDL_Color mainTextColor = {255, 255, 255, static_cast<Uint8>(255 * buttonProgress)};
        SDL_Color subTextColor = {255, 255, 255, static_cast<Uint8>(200 * buttonProgress)};
        
        renderText(btn.text, buttonRect.x + 15, buttonRect.y + 8, mainTextColor, font);
        renderText(btn.subtitle, buttonRect.x + 15, buttonRect.y + 32, subTextColor, smallFont);
    }
}

bool GameRenderer::isPointInStartButton(int x, int y) const {
    const float BOX_WIDTH = 500.0f;
    const float BUTTON_WIDTH = 180.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 40.0f;
    const float MARGIN_FROM_TOP = 30.0f;
    const float TITLE_HEIGHT = 150.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 50.0f;
    
    // Calculate the centered content group position
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y;
    const float buttonHeight = 65.0f;
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

bool GameRenderer::isPointInTutorialButton(int x, int y) const {
    const float BOX_WIDTH = 500.0f;
    const float BUTTON_WIDTH = 180.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 40.0f;
    const float MARGIN_FROM_TOP = 30.0f;
    const float TITLE_HEIGHT = 150.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 50.0f;
    const float buttonHeight = 65.0f;
    const float buttonGap = 20.0f;
    
    // Calculate the centered content group position
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y + buttonHeight + buttonGap;
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

bool GameRenderer::isPointInExitButton(int x, int y) const {
    const float BOX_WIDTH = 500.0f;
    const float BUTTON_WIDTH = 180.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 40.0f;
    const float MARGIN_FROM_TOP = 30.0f;
    const float TITLE_HEIGHT = 150.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 50.0f;
    const float buttonHeight = 65.0f;
    const float buttonGap = 20.0f;
    
    // Calculate the centered content group position
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y + 2 * (buttonHeight + buttonGap);
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

bool GameRenderer::isPointInPlayAgainButton(int x, int y) const {
    // Use same calculation as renderGameOver
    const float BOX_WIDTH = 600.0f;
    const float BUTTON_WIDTH = 200.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 50.0f;
    const float MARGIN_FROM_TOP = 40.0f;
    const float TITLE_HEIGHT = 120.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 40.0f;
    
    // Calculate the centered content group position
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y;
    const float buttonHeight = 70.0f;
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

bool GameRenderer::isPointInMainMenuButton(int x, int y) const {
    const float BOX_WIDTH = 600.0f;
    const float BUTTON_WIDTH = 200.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 50.0f;
    const float MARGIN_FROM_TOP = 40.0f;
    const float TITLE_HEIGHT = 120.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 40.0f;
    const float buttonHeight = 70.0f;
    const float buttonGap = 25.0f;
    
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y + 1 * (buttonHeight + buttonGap);
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

bool GameRenderer::isPointInGameOverExitButton(int x, int y) const {
    const float BOX_WIDTH = 600.0f;
    const float BUTTON_WIDTH = 200.0f;
    const float GAP_BETWEEN_BOX_BUTTONS = 50.0f;
    const float MARGIN_FROM_TOP = 40.0f;
    const float TITLE_HEIGHT = 120.0f;
    const float CONTENT_START_Y = MARGIN_FROM_TOP + TITLE_HEIGHT + 40.0f;
    const float buttonHeight = 70.0f;
    const float buttonGap = 25.0f;
    
    const float TOTAL_CONTENT_WIDTH = BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS + BUTTON_WIDTH;
    const float centerX = WINDOW_WIDTH / 2.0f;
    const float contentGroupStartX = centerX - (TOTAL_CONTENT_WIDTH / 2.0f);
    const float buttonX = contentGroupStartX + BOX_WIDTH + GAP_BETWEEN_BOX_BUTTONS;
    
    const float buttonY = CONTENT_START_Y + 2 * (buttonHeight + buttonGap);
    
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= buttonY && y <= buttonY + buttonHeight);
}

void GameRenderer::toggleTutorial() {
    tutorialAnimating = true;
    tutorialAnimationStart = SDL_GetTicks();
    tutorialVisible = !tutorialVisible;
    std::cout << "Tutorial " << (tutorialVisible ? "opening" : "closing") << " with animation..." << std::endl;
}

bool GameRenderer::isTutorialVisible() {
    return tutorialVisible;
}