#include <gui/screen1_screen/Screen1View.hpp>
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <touchgfx/widgets/BoxWithBorder.hpp>
#include <touchgfx/Color.hpp>
#include <gui/home_screen/HomeView.hpp>
#include <cmsis_os2.h>

extern uint16_t highestScore;
extern osMessageQueueId_t myQueue01Handle;
extern osMessageQueueId_t SongQueueHandle;
extern uint32_t random_number;
extern int play;

int random(int systick)
{
    static int a = 828172;
    static int b = 552619;
    static int md = 998244353;
    return int(((long long)a * systick % md * systick + b) % md);
}

const int Tetromino::relativePositions[COUNT][ROTATION_STATES][TETROMINO_TILE_COUNT][2] = {
    // I Tetromino
    {
        {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, // 0°
        {{1, -1}, {1, 0}, {1, 1}, {1, 2}}, // 90°
        {{-1, 1}, {0, 1}, {1, 1}, {2, 1}}, // 180°
        {{0, -1}, {0, 0}, {0, 1}, {0, 2}}  // 270°
    },
    // O Tetromino (no rotation needed)
    {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // 0°
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // 90°
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // 180°
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}  // 270°
    },
    // T Tetromino
    {
        {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},  // 0°
        {{0, -1}, {0, 0}, {0, 1}, {1, 0}},  // 90°
        {{0, -1}, {-1, 0}, {0, 0}, {1, 0}}, // 180°
        {{-1, 0}, {0, -1}, {0, 0}, {0, 1}}  // 270°
    },
    // S Tetromino
    {
        {{0, 0}, {1, 0}, {-1, 1}, {0, 1}}, // 0°
        {{0, -1}, {0, 0}, {1, 0}, {1, 1}}, // 90°
        {{0, 0}, {1, 0}, {-1, 1}, {0, 1}}, // 180°
        {{0, -1}, {0, 0}, {1, 0}, {1, 1}}  // 270°
    },
    // Z Tetromino
    {
        {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}, // 0°
        {{1, -1}, {0, 0}, {1, 0}, {0, 1}}, // 90°
        {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}, // 180°
        {{1, -1}, {0, 0}, {1, 0}, {0, 1}}  // 270°
    },
    // J Tetromino
    {
        {{-1, 0}, {-1, 1}, {0, 0}, {1, 0}}, // 0°
        {{0, -1}, {0, 0}, {0, 1}, {1, 1}},  // 90°
        {{-1, 0}, {0, 0}, {1, 0}, {1, -1}}, // 180°
        {{-1, -1}, {0, -1}, {0, 0}, {0, 1}} // 270°
    },
    // L Tetromino
    {
        {{-1, 0}, {0, 0}, {1, 0}, {1, 1}},   // 0°
        {{0, -1}, {0, 0}, {0, 1}, {1, -1}},  // 90°
        {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, // 180°
        {{-1, 1}, {0, -1}, {0, 0}, {0, 1}}   // 270°
    }};

// Constructor
Tetromino::Tetromino()
    : type(TetrominoType::I), posX(0), posY(0), tileSize(10), rotationState(0)
{
}

// Initialize the tetromino with type and starting position
void Tetromino::initialize(TetrominoType initType, int startX, int startY, int size, TetriTileID freeBoxes)
{
    type = initType;
    posX = startX;
    posY = startY;
    tileSize = size;
    rotationState = 0;

    // Định nghĩa màu sắc cho từng loại Tetromino (màu Tetris cổ điển)
    uint32_t tetrominoColor, borderColor;
    switch (initType)
    {
    case TetrominoType::I: // Cyan/Xanh nhạt
        tetrominoColor = touchgfx::Color::getColorFromRGB(0, 255, 255);
        borderColor = touchgfx::Color::getColorFromRGB(0, 200, 200);
        break;
    case TetrominoType::O: // Vàng
        tetrominoColor = touchgfx::Color::getColorFromRGB(255, 255, 0);
        borderColor = touchgfx::Color::getColorFromRGB(200, 200, 0);
        break;
    case TetrominoType::T: // Tím/Magenta
        tetrominoColor = touchgfx::Color::getColorFromRGB(128, 0, 128);
        borderColor = touchgfx::Color::getColorFromRGB(100, 0, 100);
        break;
    case TetrominoType::S: // Xanh lá
        tetrominoColor = touchgfx::Color::getColorFromRGB(0, 255, 0);
        borderColor = touchgfx::Color::getColorFromRGB(0, 200, 0);
        break;
    case TetrominoType::Z: // Đỏ
        tetrominoColor = touchgfx::Color::getColorFromRGB(255, 0, 0);
        borderColor = touchgfx::Color::getColorFromRGB(200, 0, 0);
        break;
    case TetrominoType::J: // Xanh dương
        tetrominoColor = touchgfx::Color::getColorFromRGB(0, 0, 255);
        borderColor = touchgfx::Color::getColorFromRGB(0, 0, 200);
        break;
    case TetrominoType::L: // Cam
        tetrominoColor = touchgfx::Color::getColorFromRGB(255, 165, 0);
        borderColor = touchgfx::Color::getColorFromRGB(200, 130, 0);
        break;
    default:
        tetrominoColor = touchgfx::Color::getColorFromRGB(128, 128, 128);
        borderColor = touchgfx::Color::getColorFromRGB(100, 100, 100);
        break;
    }

    for (int i = 0; i < TETROMINO_TILE_COUNT; i++)
    {
        tiles[i] = freeBoxes.tiles[i];
        tiles[i]->setColor(tetrominoColor);
        tiles[i]->setBorderColor(borderColor);
        tiles[i]->setBorderSize(2);
    }
    updateTilePositions();
}

// Move the tetromino by (dx, dy)
void Tetromino::move(int dx, int dy)
{
    posX += dx;
    posY += dy;
    updateTilePositions();
}

// Rotate the tetromino clockwise
void Tetromino::rotate()
{
    // Do not rotate the O tetromino
    if (type == TetrominoType::O)
        return;

    rotationState = (rotationState + 1) % ROTATION_STATES;
    updateTilePositions();
}

// Get a pointer to a specific tile
touchgfx::BoxWithBorder *Tetromino::getTile(int index)
{
    if (index < 0 || index >= TETROMINO_TILE_COUNT)
        return nullptr;
    return tiles[index];
}

// Update tile positions based on current rotation and central position
void Tetromino::updateTilePositions()
{
    for (int i = 0; i < TETROMINO_TILE_COUNT; ++i)
    {

        int relX = relativePositions[static_cast<int>(type)][rotationState][i][0];
        int relY = relativePositions[static_cast<int>(type)][rotationState][i][1];
        tiles[i]->setPosition(posX + relX * tileSize, posY + relY * tileSize, tileSize, tileSize);
    }
}

// Reset tetromino to uninitialized state
void Tetromino::reset()
{
    type = TetrominoType::I;
    posX = 0;
    posY = 0;
    rotationState = 0;
    for (int i = 0; i < TETROMINO_TILE_COUNT; ++i)
    {
        tiles[i]->setVisible(false);
        tiles[i]->setPosition(0, 0, tileSize, tileSize);
    }
}

// Get current tiles
TetriTileID Tetromino::getTiles()
{
    TetriTileID freeBoxes;
    int freeBoxesCount = 0;
    for (int i = 0; i < getTileCount(); i++)
    {
        freeBoxes.tiles[freeBoxesCount++] = tiles[i];
    }
    return freeBoxes;
}

Screen1View::Screen1View()
    : tickCount(0),
      subTickCount(0),
      tetrominoActive(false),
      currentScore(0),
      timerCount(0)
{
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        add(boxes[i]);
        boxes[i].setVisible(false); // Initialize all boxes
    }
    Unicode::snprintf(scoreBuffer1, SCOREBUFFER1_SIZE, "%u", currentScore);
    Unicode::snprintf(scoreBuffer2, SCOREBUFFER2_SIZE, "%u", highestScore);
    uint8_t res;
    if (osMessageQueueGetCount(myQueue01Handle) > 0)
    {
        osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);
    }
}

Screen1View::~Screen1View()
{
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    CreateGrid();
    CreateNewTetromino(true);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::gameOver()
{
    // Reset button
    RestartButton.setVisible(true);
    left.setVisible(false);
    right.setVisible(false);
    down.setVisible(false);
    rotate.setVisible(false);
    MoveHome.setVisible(false);

    // Reset tiles
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        boxes[i].setVisible(false);
    }

    uint32_t count = osMessageQueueGetCount(SongQueueHandle);
    if (count < 2)
    {
        uint8_t data = '4';
        osMessageQueuePut(SongQueueHandle, &data, 0, 10);
    }
}

void Screen1View::CreateGrid()
{
    // Create vertical grid lines
    for (int i = 0; i < MAX_LINES; i++)
    {
        int x = 2 * step * (i + 1) - 1;
        lines[i].setPosition(x, 0, 2, maxY);
        lines[i].setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
        lines[i].setBorderColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
        lines[i].setAlpha(20);
        lines[i].setBorderSize(1);
        add(lines[i]);
        lines[i].moveTo(x, 0);
    }
    invalidate();
}

TetrominoType Screen1View::getRandomStarterTetrominoType()
{
    return tetrominoTypes[random(timerCount + random_number * 3) % COUNT];
    //	return tetrominoTypes[(timerCount + 3 * random_number + highestScore) % COUNT];
}

TetrominoType Screen1View::getRandomTetrominoType()
{
    return tetrominoTypes[random(timerCount + random_number) % COUNT];
    //    return tetrominoTypes[(tickCount + timerCount + highestScore + random_number) % COUNT];
}

TetriTileID Screen1View::findFreeTiles()
{
    TetriTileID freeBoxes;
    int freeBoxesCount = 0;
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        if (!boxes[i].isVisible())
        {
            freeBoxes.tiles[freeBoxesCount++] = &boxes[i];
            boxes[i].setVisible(true); // Update active boxes
            if (freeBoxesCount == TETROMINO_TILE_COUNT)
                break;
        }
    }
    return freeBoxes;
}

void Screen1View::CreateNewTetromino(bool starter)
{
    if (tetrominoActive || RestartButton.isVisible())
        return; // Already an active tetromino

    play = 1;

    int startX = maxX / 2;
    int startY = 0;

    TetrominoType type = getRandomTetrominoType();
    if (starter)
    {
        // Initialize the first tetromino
        nextTetromino.initialize(getRandomStarterTetrominoType(), nextStartX, nextStartY, step, findFreeTiles());
    }
    // Reset currentTetromino
    currentTetromino.initialize(nextTetromino.getType(), startX, startY, step, nextTetromino.getTiles());
    nextTetromino.initialize(type, nextStartX, nextStartY, step, findFreeTiles());
    // GAMEOVER
    if (CheckCollision(startX, startY, currentTetromino))
    {
        play = 0;

        Screen1View::gameOver();
        invalidate();
        return; // Not tetrominoActive
    }

    tetrominoActive = true; // Reset tetrominoActive
    invalidate();
}

bool Screen1View::CheckCollision(int newX, int newY, Tetromino &tetromino)
{
    for (int i = 0; i < tetromino.getTileCount(); ++i)
    {
        BoxWithBorder *tile = tetromino.getTile(i);
        int x = tile->getX();
        int y = tile->getY();

        // Check boundaries
        if (x < 0 || (x + tile->getWidth()) > maxX || (y + tile->getHeight()) > maxY)
        {
            return true;
        }

        // Check collision with active blocks
        for (int j = 0; j < MAX_BLOCKS; ++j)
        {

            if (!boxes[j].isVisible())
                continue; // Skip not active boxes
            touchgfx::BoxWithBorder *active = &boxes[j];
            if (active == tile)
                continue; // Skip the same boxes

            if (!(x + tile->getWidth() <= active->getX() ||
                  active->getX() + active->getWidth() <= x ||
                  y + tile->getHeight() <= active->getY() ||
                  active->getY() + active->getHeight() <= y))
            {
                return true;
            }
        }
    }
    return false;
}

bool Screen1View::MoveTetromino(int stepx, int stepy)
{
    if (!tetrominoActive)
        return false;

    // Move tetromino
    currentTetromino.move(stepx, stepy);

    // Check collision
    if (CheckCollision(currentTetromino.getPosX(), currentTetromino.getPosY(), currentTetromino))
    {
        // Move back if collision detected
        currentTetromino.move(-stepx, -stepy);
        return false;
    }

    invalidate();
    return true;
}

void Screen1View::RotateTetromino()
{
    if (!tetrominoActive)
        return;

    // Rotate tetromino
    currentTetromino.rotate();

    // Check collision after rotation
    if (CheckCollision(currentTetromino.getPosX(), currentTetromino.getPosY(), currentTetromino))
    {
        // Rotate back if collision detected
        for (int i = 0; i < 3; ++i)
        { // Rotate 3 more times to go back
            currentTetromino.rotate();
        }
    }

    invalidate();
}

void Screen1View::Move()
{
    uint8_t res;
    if (osMessageQueueGetCount(myQueue01Handle) > 0)
    {
        osMessageQueueGet(myQueue01Handle, &res, NULL, osWaitForever);
        if (!tetrominoActive)
            changeScreen();
        if (res == 'L')
        {
            MoveTetromino(-step, 0);
        }
        else if (res == 'R')
        {
            MoveTetromino(step, 0);
        }
        else if (res == 'O')
        {
            RotateTetromino();
        }
        else if (res == 'D')
        {
            while (MoveTetromino(0, step))
            {
            }
        }
    }
}

void Screen1View::handleTickEvent()
{
    subTickCount++;
    timerCount++;
    Move();
    if (subTickCount >= currentSubTickCount)
    { // Increase this to slow down
        subTickCount = 0;
        tickCount++;
        if (!tetrominoActive)
            return;
        if (!MoveTetromino(0, step))
        {
            // Fix the current tetromino blocks
            bool checkFullRows[MAX_ROWS] = {};
            bool checkErase = false;

            uint32_t count = osMessageQueueGetCount(SongQueueHandle);
            if (count < 2)
            {
                uint8_t data = '2';
                osMessageQueuePut(SongQueueHandle, &data, 0, 10);
            }

            for (int i = 0; i < currentTetromino.getTileCount(); ++i)
            {
                BoxWithBorder *tile = currentTetromino.getTile(i);
                tile->setColor(touchgfx::Color::getColorFromRGB(200, 200, 200)); // Fixed block color
                int row = tile->getY() / step;
                rows[row] += 1;
                if (rows[row] == MAX_BLOCK_PER_ROW)
                {
                    checkFullRows[row] = true;
                    checkErase = true;
                }
            }

            // Erase full rows
            if (checkErase)
                eraseFullRows(checkFullRows);

            tetrominoActive = false;
            CreateNewTetromino(false);
        }

        // Speed up
        if (tickCount > updateSpeedTickCount)
        {
            tickCount = 0;
            speedUp();
        }
    }
}

void Screen1View::eraseFullRows(bool *checkFullRows)
{
    uint8_t deltas[MAX_ROWS]; // The number of tiles to move down for each row
    deltas[MAX_ROWS - 1] = 0;
    for (int i = MAX_ROWS - 2; i >= 0; i--)
    {
        if (checkFullRows[i + 1])
            deltas[i] = deltas[i + 1] + 1;
        else
            deltas[i] = deltas[i + 1];
    }

    TetriTileID nextTetrominoTiles = nextTetromino.getTiles();
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        // Skip non-active boxes and nextTetrominoTiles
        if (!boxes[i].isVisible() || nextTetrominoTiles.checkEqual(&(boxes[i])))
            continue;

        // Update position
        touchgfx::BoxWithBorder *active = &boxes[i];
        uint8_t row = active->getY() / step;
        if (checkFullRows[row])
        { // Update new non-active boxes
            active->setVisible(false);
            rows[row]--;
        }
        if (active->isVisible())
        { // Update active boxes
            active->moveTo(active->getX(), active->getY() + deltas[row] * step);
            rows[row]--;
            rows[row + deltas[row]]++;
        }
    }

    // Update the score
    currentScore += deltas[0] * deltas[0]; // deltas[0] is the number of rows deleted
    Unicode::snprintf(scoreBuffer1, SCOREBUFFER1_SIZE, "%u", currentScore);
    if (currentScore > highestScore)
    {
        highestScore = currentScore;
        touchgfx::Unicode::snprintf(scoreBuffer2, SCOREBUFFER2_SIZE, "%u", currentScore);
    }
    uint32_t count = osMessageQueueGetCount(SongQueueHandle);
    if (count < 2)
    {
        uint8_t data = '3';
        osMessageQueuePut(SongQueueHandle, &data, 0, 10);
    }
    invalidate();
}

void Screen1View::MoveDown()
{
    // Rapidly move down to the lowest possible position
    while (MoveTetromino(0, step))
    {
    }
}

void Screen1View::MoveLeft()
{
    MoveTetromino(-step, 0);
}

void Screen1View::MoveRight()
{
    MoveTetromino(step, 0);
}

void Screen1View::Rotate()
{
    RotateTetromino();
}
