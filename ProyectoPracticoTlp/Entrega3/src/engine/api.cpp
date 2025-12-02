#include "engine/api.h"

// SDL 1.2
#include <SDL.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>

namespace Engine {

    // Ventana fija 640x480
    static const int WINDOW_WIDTH  = 640;
    static const int WINDOW_HEIGHT = 480;

    // Control de tiempo para Tetris
    static Uint32 gLastTetrisDrop = 0;
    static const Uint32 TETRIS_DROP_INTERVAL = 1000; // 1 segundo

    // Estructura para una celda de pieza de Tetris
    struct TetrisCell {
        int offsetX;
        int offsetY;
    };

    // Definiciones de formas de Tetris (rotación 0)
    struct TetrisShape {
        std::string name;
        std::vector<TetrisCell> cells;
        Uint8 r, g, b;
    };

    static std::vector<TetrisShape> gTetrisShapes;

    struct Entity {
        int id;
        int gx;
        int gy;
        int w;
        int h;
        std::string type;
        int shapeIndex; // Para tetris
        int rotation;   // 0, 1, 2, 3
    };

    static SDL_Surface* gScreen = 0;

    static std::vector<Entity> gEntities;
    static int  gNextId    = 1;
    static int  gScore     = 0;
    static bool gGameEnded = false;

    static int gTetrisId   = -1;
    static int gSnakeId    = -1;

    static std::vector<int> gSnakeSegments;
    static int gSnakeDirX  = 1;
    static int gSnakeDirY  = 0;

    // Tablero de Tetris para detectar colisiones y líneas completas
    static int gTetrisBoard[BOARD_HEIGHT][BOARD_WIDTH];

    // ---------------------------------------------------------------------
    // Sistema simple de dígitos (7 segmentos)
    // ---------------------------------------------------------------------
    static void drawDigit(int digit, int x, int y, int w, int h, Uint32 color) {
        if (!gScreen) return;
        
        // Segmentos: top, topRight, topLeft, middle, bottomRight, bottomLeft, bottom
        bool segments[10][7] = {
            {1,1,1,0,1,1,1}, // 0
            {0,1,0,0,1,0,0}, // 1
            {1,1,0,1,0,1,1}, // 2
            {1,1,0,1,1,0,1}, // 3
            {0,1,1,1,1,0,0}, // 4
            {1,0,1,1,1,0,1}, // 5
            {1,0,1,1,1,1,1}, // 6
            {1,1,0,0,1,0,0}, // 7
            {1,1,1,1,1,1,1}, // 8
            {1,1,1,1,1,0,1}  // 9
        };

        if (digit < 0 || digit > 9) return;

        int segW = w / 5;
        int segH = h / 10;

        SDL_Rect rect;

        // Top
        if (segments[digit][0]) {
            rect.x = x + segW;
            rect.y = y;
            rect.w = w - 2*segW;
            rect.h = segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Top right
        if (segments[digit][1]) {
            rect.x = x + w - segW;
            rect.y = y + segH;
            rect.w = segW;
            rect.h = h/2 - 2*segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Top left
        if (segments[digit][2]) {
            rect.x = x;
            rect.y = y + segH;
            rect.w = segW;
            rect.h = h/2 - 2*segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Middle
        if (segments[digit][3]) {
            rect.x = x + segW;
            rect.y = y + h/2 - segH/2;
            rect.w = w - 2*segW;
            rect.h = segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Bottom right
        if (segments[digit][4]) {
            rect.x = x + w - segW;
            rect.y = y + h/2 + segH;
            rect.w = segW;
            rect.h = h/2 - 2*segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Bottom left
        if (segments[digit][5]) {
            rect.x = x;
            rect.y = y + h/2 + segH;
            rect.w = segW;
            rect.h = h/2 - 2*segH;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Bottom
        if (segments[digit][6]) {
            rect.x = x + segW;
            rect.y = y + h - segH;
            rect.w = w - 2*segW;
            rect.h = segH;
            SDL_FillRect(gScreen, &rect, color);
        }
    }

    static void drawScore(int score, int x, int y) {
        if (!gScreen) return;

        std::string scoreStr = std::to_string(score);
        int digitWidth = 20;
        int digitHeight = 30;
        int spacing = 5;

        Uint32 white = SDL_MapRGB(gScreen->format, 255, 255, 255);

        for (size_t i = 0; i < scoreStr.length(); ++i) {
            int digit = scoreStr[i] - '0';
            drawDigit(digit, x + i * (digitWidth + spacing), y, digitWidth, digitHeight, white);
        }
    }

    // ---------------------------------------------------------------------
    // Inicializar formas de Tetris
    // ---------------------------------------------------------------------
    static void initTetrisShapes() {
        gTetrisShapes.clear();

        // Pieza I (cyan)
        {
            TetrisShape shape;
            shape.name = "I";
            shape.r = 0; shape.g = 255; shape.b = 255;
            shape.cells.push_back({0, 0});
            shape.cells.push_back({1, 0});
            shape.cells.push_back({2, 0});
            shape.cells.push_back({3, 0});
            gTetrisShapes.push_back(shape);
        }

        // Pieza O (amarillo)
        {
            TetrisShape shape;
            shape.name = "O";
            shape.r = 255; shape.g = 255; shape.b = 0;
            shape.cells.push_back({0, 0});
            shape.cells.push_back({1, 0});
            shape.cells.push_back({0, 1});
            shape.cells.push_back({1, 1});
            gTetrisShapes.push_back(shape);
        }

        // Pieza T (morado)
        {
            TetrisShape shape;
            shape.name = "T";
            shape.r = 128; shape.g = 0; shape.b = 128;
            shape.cells.push_back({1, 0});
            shape.cells.push_back({0, 1});
            shape.cells.push_back({1, 1});
            shape.cells.push_back({2, 1});
            gTetrisShapes.push_back(shape);
        }

        // Pieza L (naranja)
        {
            TetrisShape shape;
            shape.name = "L";
            shape.r = 255; shape.g = 165; shape.b = 0;
            shape.cells.push_back({2, 0});
            shape.cells.push_back({0, 1});
            shape.cells.push_back({1, 1});
            shape.cells.push_back({2, 1});
            gTetrisShapes.push_back(shape);
        }

        // Pieza J (azul)
        {
            TetrisShape shape;
            shape.name = "J";
            shape.r = 0; shape.g = 0; shape.b = 255;
            shape.cells.push_back({0, 0});
            shape.cells.push_back({0, 1});
            shape.cells.push_back({1, 1});
            shape.cells.push_back({2, 1});
            gTetrisShapes.push_back(shape);
        }

        // Pieza S (verde)
        {
            TetrisShape shape;
            shape.name = "S";
            shape.r = 0; shape.g = 255; shape.b = 0;
            shape.cells.push_back({1, 0});
            shape.cells.push_back({2, 0});
            shape.cells.push_back({0, 1});
            shape.cells.push_back({1, 1});
            gTetrisShapes.push_back(shape);
        }

        // Pieza Z (rojo)
        {
            TetrisShape shape;
            shape.name = "Z";
            shape.r = 255; shape.g = 0; shape.b = 0;
            shape.cells.push_back({0, 0});
            shape.cells.push_back({1, 0});
            shape.cells.push_back({1, 1});
            shape.cells.push_back({2, 1});
            gTetrisShapes.push_back(shape);
        }
    }

    // ---------------------------------------------------------------------
    // Rotar celdas de Tetris
    // ---------------------------------------------------------------------
    static std::vector<TetrisCell> rotateCells(const std::vector<TetrisCell>& cells, int rotation) {
        std::vector<TetrisCell> result = cells;
        
        for (int r = 0; r < rotation; ++r) {
            for (size_t i = 0; i < result.size(); ++i) {
                int oldX = result[i].offsetX;
                int oldY = result[i].offsetY;
                // Rotación 90° horario: (x, y) -> (y, -x)
                result[i].offsetX = oldY;
                result[i].offsetY = -oldX;
            }
        }
        
        return result;
    }

    // ---------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------

    static Entity* findEntity(int id) {
        for (size_t i = 0; i < gEntities.size(); ++i) {
            if (gEntities[i].id == id)
                return &gEntities[i];
        }
        return 0;
    }

    static bool isTetrisType(const std::string& t) {
        return (
            t == "I" || t == "O" || t == "T" ||
            t == "L" || t == "J" || t == "S" ||
            t == "Z" || t == "Tetris" || t == "Block"
        );
    }

    static bool isSnakeHeadType(const std::string& t) {
        return (t == "Snake");
    }

    static bool isSnakeBodyType(const std::string& t) {
        return (t == "SnakeBody");
    }

    static bool isSnakeAnyType(const std::string& t) {
        return isSnakeHeadType(t) || isSnakeBodyType(t);
    }

    static bool isFoodType(const std::string& t) {
        return (t == "Food");
    }

    static bool hasFood() {
        for (size_t i = 0; i < gEntities.size(); ++i) {
            if (isFoodType(gEntities[i].type))
                return true;
        }
        return false;
    }

    static void placeFoodRandom(Entity &food) {
        int attempts = 0;
        for (;;) {
            int x = std::rand() % BOARD_WIDTH;
            int y = std::rand() % BOARD_HEIGHT;

            bool onSnake = false;
            for (size_t i = 0; i < gSnakeSegments.size(); ++i) {
                Entity* seg = findEntity(gSnakeSegments[i]);
                if (seg && seg->gx == x && seg->gy == y) {
                    onSnake = true;
                    break;
                }
            }
            if (!onSnake) {
                food.gx = x;
                food.gy = y;
                return;
            }
            if (++attempts > 100) {
                food.gx = 0;
                food.gy = 0;
                return;
            }
        }
    }

    static void ensureFoodExists() {
        if (hasFood()) return;
        Entity f;
        f.id   = gNextId++;
        f.w    = 1;
        f.h    = 1;
        f.type = "Food";
        f.shapeIndex = -1;
        f.rotation = 0;
        placeFoodRandom(f);
        gEntities.push_back(f);
        std::cout << "[Engine] ensureFoodExists -> Food id="
                  << f.id << " at (" << f.gx << "," << f.gy << ")\n";
    }

    // ---------------------------------------------------------------------
    // TETRIS - Verificar colisión
    // ---------------------------------------------------------------------
    static bool checkTetrisCollision(int gx, int gy, int shapeIndex, int rotation) {
        if (shapeIndex < 0 || shapeIndex >= (int)gTetrisShapes.size())
            return true;

        std::vector<TetrisCell> cells = rotateCells(gTetrisShapes[shapeIndex].cells, rotation);

        for (size_t i = 0; i < cells.size(); ++i) {
            int cx = gx + cells[i].offsetX;
            int cy = gy + cells[i].offsetY;

            if (cx < 0 || cx >= BOARD_WIDTH || cy < 0 || cy >= BOARD_HEIGHT)
                return true;

            if (gTetrisBoard[cy][cx] != 0)
                return true;
        }

        return false;
    }

    // ---------------------------------------------------------------------
    // TETRIS - Fijar pieza en el tablero
    // ---------------------------------------------------------------------
    static void fixTetrisPiece(Entity* e) {
        if (!e || e->shapeIndex < 0) return;

        std::vector<TetrisCell> cells = rotateCells(gTetrisShapes[e->shapeIndex].cells, e->rotation);

        for (size_t i = 0; i < cells.size(); ++i) {
            int cx = e->gx + cells[i].offsetX;
            int cy = e->gy + cells[i].offsetY;

            if (cx >= 0 && cx < BOARD_WIDTH && cy >= 0 && cy < BOARD_HEIGHT) {
                gTetrisBoard[cy][cx] = e->shapeIndex + 1;
            }
        }

        e->type = "Fixed";
        gTetrisId = -1;
        std::cout << "[Engine] Tetris piece fixed id=" << e->id << "\n";

        // Verificar líneas completas
        int linesCleared = 0;
        for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
            bool full = true;
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                if (gTetrisBoard[y][x] == 0) {
                    full = false;
                    break;
                }
            }

            if (full) {
                linesCleared++;
                // Mover todo hacia abajo
                for (int yy = y; yy > 0; --yy) {
                    for (int x = 0; x < BOARD_WIDTH; ++x) {
                        gTetrisBoard[yy][x] = gTetrisBoard[yy - 1][x];
                    }
                }
                // Limpiar la línea superior
                for (int x = 0; x < BOARD_WIDTH; ++x) {
                    gTetrisBoard[0][x] = 0;
                }
                y++; // Revisar la misma línea de nuevo
            }
        }

        // SOLO otorgar puntos si se completaron líneas
        if (linesCleared > 0) {
            if (linesCleared == 1) {
                addScore(100);
                std::cout << "[Engine] 1 línea completada: +100 puntos\n";
            } else if (linesCleared == 2) {
                addScore(300);
                std::cout << "[Engine] 2 líneas completadas: +300 puntos\n";
            } else if (linesCleared == 3) {
                addScore(500);
                std::cout << "[Engine] 3 líneas completadas: +500 puntos\n";
            } else if (linesCleared >= 4) {
                addScore(800);
                std::cout << "[Engine] TETRIS (4 líneas): +800 puntos\n";
            }
        }

        // Spawn nueva pieza
        int idx = std::rand() % gTetrisShapes.size();
        Entity newPiece;
        newPiece.id = gNextId++;
        newPiece.w = 1;
        newPiece.h = 1;
        newPiece.type = gTetrisShapes[idx].name;
        newPiece.shapeIndex = idx;
        newPiece.rotation = 0;
        newPiece.gx = BOARD_WIDTH / 2 - 1;
        newPiece.gy = 0;

        // Verificar game over
        if (checkTetrisCollision(newPiece.gx, newPiece.gy, newPiece.shapeIndex, newPiece.rotation)) {
            endGame("Tetris: Game Over!");
            return;
        }

        gEntities.push_back(newPiece);
        gTetrisId = newPiece.id;
        gLastTetrisDrop = SDL_GetTicks(); // Reiniciar temporizador
        std::cout << "[Engine] Nueva pieza de Tetris: " << newPiece.type << " id=" << newPiece.id << "\n";
    }

    // ---------------------------------------------------------------------
    // TETRIS - Spawn pieza inicial
    // ---------------------------------------------------------------------
    static int spawnRandomTetrisPiece() {
        int idx = std::rand() % gTetrisShapes.size();

        Entity e;
        e.id   = gNextId++;
        e.w    = 1;
        e.h    = 1;
        e.type = gTetrisShapes[idx].name;
        e.shapeIndex = idx;
        e.rotation = 0;
        e.gx   = BOARD_WIDTH / 2 - 1;
        e.gy   = 0;

        gEntities.push_back(e);
        gTetrisId = e.id;
        gLastTetrisDrop = SDL_GetTicks(); // Inicializar temporizador

        std::cout << "[Engine] spawnRandomTetrisPiece type=" << e.type
                  << " id=" << e.id << " at (" << e.gx << "," << e.gy << ")\n";
        return e.id;
    }

    // ---------------------------------------------------------------------
    // TETRIS - Actualizar caída automática
    // ---------------------------------------------------------------------
    static void updateTetrisAutoDrop() {
        if (gTetrisId == -1) return;

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - gLastTetrisDrop >= TETRIS_DROP_INTERVAL) {
            Entity* e = findEntity(gTetrisId);
            if (e) {
                if (!checkTetrisCollision(e->gx, e->gy + 1, e->shapeIndex, e->rotation)) {
                    e->gy++;
                } else {
                    fixTetrisPiece(e);
                }
            }
            gLastTetrisDrop = currentTime;
        }
    }

    // ---------------------------------------------------------------------
    // Inicialización / apagado (SDL 1.2)
    // ---------------------------------------------------------------------

    void initEngine() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "[Engine] SDL_Init error: " << SDL_GetError() << "\n";
            std::exit(1);
        }

        gScreen = SDL_SetVideoMode(
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            32,
            SDL_HWSURFACE | SDL_DOUBLEBUF
        );

        if (!gScreen) {
            std::cerr << "[Engine] SDL_SetVideoMode error: "
                      << SDL_GetError() << "\n";
            SDL_Quit();
            std::exit(1);
        }

        SDL_WM_SetCaption("Proyecto TLP - Motor", 0);

        std::srand(static_cast<unsigned>(std::time(0)));

        initTetrisShapes();

        gEntities.clear();
        gSnakeSegments.clear();
        gNextId    = 1;
        gScore     = 0;
        gGameEnded = false;
        gTetrisId  = -1;
        gSnakeId   = -1;
        gSnakeDirX = 1;
        gSnakeDirY = 0;
        gLastTetrisDrop = 0;

        // Limpiar tablero de Tetris
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                gTetrisBoard[y][x] = 0;
            }
        }

        std::cout << "[Engine] initEngine() - SDL 1.2 initialized\n";
    }

    void shutdownEngine() {
        std::cout << "[Engine] shutdownEngine()\n";
        gEntities.clear();
        gSnakeSegments.clear();
        if (gScreen) {
            SDL_FreeSurface(gScreen);
            gScreen = 0;
        }
        SDL_Quit();
    }

    // ---------------------------------------------------------------------
    // Eventos
    // ---------------------------------------------------------------------

    bool pollEvents() {
        // Actualizar caída automática de Tetris
        updateTetrisAutoDrop();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return false;
            }
            if (e.type == SDL_KEYDOWN) {
                SDLKey key = e.key.keysym.sym;
                switch (key) {
                    case SDLK_ESCAPE:
                        return false;

                    case SDLK_LEFT:
                        if (gTetrisId != -1) {
                            Entity* ent = findEntity(gTetrisId);
                            if (ent && !checkTetrisCollision(ent->gx - 1, ent->gy, ent->shapeIndex, ent->rotation)) {
                                ent->gx--;
                            }
                        }
                        break;

                    case SDLK_RIGHT:
                        if (gTetrisId != -1) {
                            Entity* ent = findEntity(gTetrisId);
                            if (ent && !checkTetrisCollision(ent->gx + 1, ent->gy, ent->shapeIndex, ent->rotation)) {
                                ent->gx++;
                            }
                        }
                        break;

                    case SDLK_DOWN:
                        if (gTetrisId != -1) {
                            Entity* ent = findEntity(gTetrisId);
                            if (ent) {
                                if (!checkTetrisCollision(ent->gx, ent->gy + 1, ent->shapeIndex, ent->rotation)) {
                                    ent->gy++;
                                    gLastTetrisDrop = SDL_GetTicks(); // Reiniciar temporizador al acelerar
                                } else {
                                    fixTetrisPiece(ent);
                                }
                            }
                        }
                        break;

                    case SDLK_UP:
                        if (gTetrisId != -1) {
                            Entity* ent = findEntity(gTetrisId);
                            if (ent) {
                                int newRot = (ent->rotation + 1) % 4;
                                if (!checkTetrisCollision(ent->gx, ent->gy, ent->shapeIndex, newRot)) {
                                    ent->rotation = newRot;
                                }
                            }
                        }
                        break;

                    case SDLK_w:
                        if (gSnakeId != -1 && gSnakeDirY == 0) {
                            gSnakeDirX = 0;
                            gSnakeDirY = -1;
                        }
                        break;

                    case SDLK_s:
                        if (gSnakeId != -1 && gSnakeDirY == 0) {
                            gSnakeDirX = 0;
                            gSnakeDirY = 1;
                        }
                        break;

                    case SDLK_a:
                        if (gSnakeId != -1 && gSnakeDirX == 0) {
                            gSnakeDirX = -1;
                            gSnakeDirY = 0;
                        }
                        break;

                    case SDLK_d:
                        if (gSnakeId != -1 && gSnakeDirX == 0) {
                            gSnakeDirX = 1;
                            gSnakeDirY = 0;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
        return !gGameEnded;
    }

    // ---------------------------------------------------------------------
    // Dibujo (SDL 1.2)
    // ---------------------------------------------------------------------

    void presentFrame() {
        if (!gScreen) return;

        // Borrar a negro
        Uint32 black = SDL_MapRGB(gScreen->format, 0, 0, 0);
        SDL_FillRect(gScreen, 0, black);

        int tileW = WINDOW_WIDTH  / BOARD_WIDTH;
        int tileH = WINDOW_HEIGHT / BOARD_HEIGHT;

        // Dibujar tablero de Tetris fijo
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            for (int x = 0; x < BOARD_WIDTH; ++x) {
                if (gTetrisBoard[y][x] > 0) {
                    int shapeIdx = gTetrisBoard[y][x] - 1;
                    if (shapeIdx >= 0 && shapeIdx < (int)gTetrisShapes.size()) {
                        Uint32 color = SDL_MapRGB(gScreen->format,
                            gTetrisShapes[shapeIdx].r,
                            gTetrisShapes[shapeIdx].g,
                            gTetrisShapes[shapeIdx].b);

                        SDL_Rect rect;
                        rect.x = x * tileW;
                        rect.y = y * tileH;
                        rect.w = tileW - 1;
                        rect.h = tileH - 1;
                        SDL_FillRect(gScreen, &rect, color);
                    }
                }
            }
        }

        // Dibujar entidades
        for (size_t i = 0; i < gEntities.size(); ++i) {
            const Entity &ent = gEntities[i];

            if (ent.type == "Fixed") continue; // Ya dibujado en el tablero

            Uint32 color;

            // Tetris activo
            if (isTetrisType(ent.type) && ent.shapeIndex >= 0) {
                std::vector<TetrisCell> cells = rotateCells(
                    gTetrisShapes[ent.shapeIndex].cells,
                    ent.rotation
                );

                color = SDL_MapRGB(gScreen->format,
                    gTetrisShapes[ent.shapeIndex].r,
                    gTetrisShapes[ent.shapeIndex].g,
                    gTetrisShapes[ent.shapeIndex].b);

                for (size_t c = 0; c < cells.size(); ++c) {
                    int cx = ent.gx + cells[c].offsetX;
                    int cy = ent.gy + cells[c].offsetY;

                    if (cx >= 0 && cx < BOARD_WIDTH && cy >= 0 && cy < BOARD_HEIGHT) {
                        SDL_Rect rect;
                        rect.x = cx * tileW;
                        rect.y = cy * tileH;
                        rect.w = tileW - 1;
                        rect.h = tileH - 1;
                        SDL_FillRect(gScreen, &rect, color);
                    }
                }
                continue;
            }

            // Snake
            if (isSnakeHeadType(ent.type)) {
                color = SDL_MapRGB(gScreen->format, 0, 200, 0);
            } else if (isSnakeBodyType(ent.type)) {
                color = SDL_MapRGB(gScreen->format, 0, 150, 0);
            } else if (isFoodType(ent.type)) {
                color = SDL_MapRGB(gScreen->format, 255, 0, 0);
            } else {
                color = SDL_MapRGB(gScreen->format, 128, 128, 128);
            }

            SDL_Rect rect;
            rect.x = ent.gx * tileW;
            rect.y = ent.gy * tileH;
            rect.w = ent.w * tileW - 1;
            rect.h = ent.h * tileH - 1;
            SDL_FillRect(gScreen, &rect, color);
        }

        // Dibujar puntuación con sistema de 7 segmentos
        drawScore(gScore, 10, 10);

        SDL_Flip(gScreen);
    }

    // ---------------------------------------------------------------------
    // Score
    // ---------------------------------------------------------------------

    void setScore(int value) {
        gScore = value;
        std::cout << "[Engine] setScore " << gScore << "\n";
    }

    void addScore(int delta) {
        gScore += delta;
        std::cout << "[Engine] addScore " << delta
                  << " => " << gScore << "\n";
    }

    // ---------------------------------------------------------------------
    // Spawn de entidades (Tetris + Snake)
    // ---------------------------------------------------------------------

    int spawnBlock(const std::string& typeIn, int gridX, int gridY) {
        std::string t = typeIn;

        // Si es tipo Tetris
        if (isTetrisType(t)) {
            if (gTetrisId != -1) {
                std::cout << "[Engine] spawnBlock Tetris piece already exists\n";
                return gTetrisId;
            }
            return spawnRandomTetrisPiece();
        }

        // Si es Snake
        if (gSnakeId == -1) {
            Entity head;
            head.id   = gNextId++;
            head.gx   = gridX;
            head.gy   = gridY;
            head.w    = 1;
            head.h    = 1;
            head.type = "Snake";
            head.shapeIndex = -1;
            head.rotation = 0;
            gEntities.push_back(head);
            gSnakeId = head.id;
            gSnakeSegments.clear();
            gSnakeSegments.push_back(head.id);

            std::cout << "[Engine] spawnBlock Snake id=" << head.id
                      << " at (" << head.gx << "," << head.gy << ")\n";

            ensureFoodExists();
            return head.id;
        }

        // Food
        Entity f;
        f.id   = gNextId++;
        f.w    = 1;
        f.h    = 1;
        f.type = "Food";
        f.shapeIndex = -1;
        f.rotation = 0;
        placeFoodRandom(f);
        gEntities.push_back(f);

        std::cout << "[Engine] spawnBlock -> Food id=" << f.id
                  << " at (" << f.gx << "," << f.gy << ")\n";
        return f.id;
    }

    // ---------------------------------------------------------------------
    // Movimiento
    // ---------------------------------------------------------------------

    void moveEntity(int id, int dx, int dy) {
        Entity* e = findEntity(id);
        if (!e) return;

        if (e->type == "Fixed") return;

        // Tetris
        if (isTetrisType(e->type) && e->shapeIndex >= 0) {
            int newGx = e->gx + dx;
            int newGy = e->gy + dy;

            if (!checkTetrisCollision(newGx, newGy, e->shapeIndex, e->rotation)) {
                e->gx = newGx;
                e->gy = newGy;
            } else if (dy > 0) {
                fixTetrisPiece(e);
            }
            return;
        }

        // Snake cabeza
        if (isSnakeHeadType(e->type)) {
            int newGx = e->gx + gSnakeDirX;
            int newGy = e->gy + gSnakeDirY;

            // Wrap-around en los bordes
            if (newGx < 0) newGx = BOARD_WIDTH - 1;
            if (newGx >= BOARD_WIDTH) newGx = 0;
            if (newGy < 0) newGy = BOARD_HEIGHT - 1;
            if (newGy >= BOARD_HEIGHT) newGy = 0;

            // Verificar auto-colisión
            for (size_t i = 1; i < gSnakeSegments.size(); ++i) {
                Entity* seg = findEntity(gSnakeSegments[i]);
                if (seg && seg->gx == newGx && seg->gy == newGy) {
                    endGame("Snake: Auto-colisión!");
                    return;
                }
            }

            Entity* eatenFood = 0;
            bool willEat = false;
            for (size_t i = 0; i < gEntities.size(); ++i) {
                if (isFoodType(gEntities[i].type) &&
                    gEntities[i].gx == newGx && gEntities[i].gy == newGy) {
                    eatenFood = &gEntities[i];
                    willEat = true;
                    break;
                }
            }

            std::vector<std::pair<int,int>> oldPos;
            for (size_t i = 0; i < gSnakeSegments.size(); ++i) {
                Entity* seg = findEntity(gSnakeSegments[i]);
                if (seg) {
                    oldPos.push_back({seg->gx, seg->gy});
                }
            }

            e->gx = newGx;
            e->gy = newGy;

            for (size_t i = 1; i < gSnakeSegments.size(); ++i) {
                Entity* seg = findEntity(gSnakeSegments[i]);
                if (seg) {
                    seg->gx = oldPos[i - 1].first;
                    seg->gy = oldPos[i - 1].second;
                }
            }

            if (willEat && eatenFood) {
                addScore(1);  // 1 punto por fruta
                placeFoodRandom(*eatenFood);

                Entity tail;
                tail.id   = gNextId++;
                tail.w    = 1;
                tail.h    = 1;
                tail.type = "SnakeBody";
                tail.shapeIndex = -1;
                tail.rotation = 0;

                if (!oldPos.empty()) {
                    tail.gx = oldPos.back().first;
                    tail.gy = oldPos.back().second;
                } else {
                    tail.gx = e->gx;
                    tail.gy = e->gy;
                }

                gEntities.push_back(tail);
                gSnakeSegments.push_back(tail.id);

                std::cout << "[Engine] Snake ate food -> new food at ("
                          << eatenFood->gx << "," << eatenFood->gy << ")\n";
            }

            return;
        }

        // Otros
        int newGx = e->gx + dx;
        int newGy = e->gy + dy;

        if (newGx < 0) newGx = 0;
        if (newGx > BOARD_WIDTH - e->w) newGx = BOARD_WIDTH - e->w;

        if (newGy < 0) newGy = 0;
        if (newGy > BOARD_HEIGHT - e->h) newGy = BOARD_HEIGHT - e->h;

        e->gx = newGx;
        e->gy = newGy;

        std::cout << "[Engine] moveEntity id=" << id
                  << " dx=" << dx << " dy=" << dy
                  << " => (" << e->gx << "," << e->gy << ")\n";
    }

    bool isGameEnded() {
        return gGameEnded;
    }

    void rotateEntity(int id) {
        Entity* e = findEntity(id);
        if (!e || e->shapeIndex < 0) return;

        int newRot = (e->rotation + 1) % 4;
        if (!checkTetrisCollision(e->gx, e->gy, e->shapeIndex, newRot)) {
            e->rotation = newRot;
            std::cout << "[Engine] rotateEntity id=" << id << " rotation=" << newRot << "\n";
        }
    }

    void dropEntity(int id) {
        Entity* e = findEntity(id);
        if (!e || e->shapeIndex < 0) return;

        while (!checkTetrisCollision(e->gx, e->gy + 1, e->shapeIndex, e->rotation)) {
            e->gy++;
        }
        fixTetrisPiece(e);
        std::cout << "[Engine] dropEntity id=" << id << " -> bottom\n";
    }

    void endGame(const std::string& r) {
        gGameEnded = true;
        std::cout << "[Engine] endGame() called. Reason: " << r << "\n";
    }

    void drawText(const std::string& t, int x, int y) {
        std::cout << "[Engine] drawText \"" << t
                  << "\" at (" << x << "," << y << ")\n";
    }

} // namespace Engine
