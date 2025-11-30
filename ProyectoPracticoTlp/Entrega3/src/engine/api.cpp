#include "engine/api.h"

#include <SDL.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

namespace Engine {

    // Ventana fija 640x480
    static constexpr int WINDOW_WIDTH  = 640;
    static constexpr int WINDOW_HEIGHT = 480;

    struct Entity {
        int id;
        int gx;          // posición en la grilla (columna)
        int gy;          // posición en la grilla (fila)
        int w;           // ancho en tiles
        int h;           // alto  en tiles
        std::string type; // "I", "O", "T", "L", "Z", "Snake", "SnakeBody", "Food", "Fixed", etc.
    };

    static SDL_Window*   gWindow   = nullptr;
    static SDL_Renderer* gRenderer = nullptr;

    static std::vector<Entity> gEntities;
    static int  gNextId    = 1;
    static int  gScore     = 0;
    static bool gGameEnded = false;

    // IDs especiales
    static int gTetrisId   = -1;  // id de la pieza de Tetris actual
    static int gSnakeId    = -1;  // id de la cabeza de la serpiente

    // Segmentos de la serpiente: ids en orden cabeza -> cola
    static std::vector<int> gSnakeSegments;

    // Dirección actual de la serpiente (en la grilla)
    static int gSnakeDirX  = 1;   // empieza moviéndose a la derecha
    static int gSnakeDirY  = 0;

    // ---------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------

    static Entity* findEntity(int id) {
        for (auto &e : gEntities) {
            if (e.id == id) return &e;
        }
        return nullptr;
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

    // ¿Hay al menos una fruta?
    static bool hasFood() {
        for (auto &e : gEntities) {
            if (isFoodType(e.type)) return true;
        }
        return false;
    }

    // Colocar una fruta en una casilla aleatoria que no esté ocupada por la serpiente
    static void placeFoodRandom(Entity &food) {
        int attempts = 0;
        while (true) {
            int x = std::rand() % BOARD_WIDTH;
            int y = std::rand() % BOARD_HEIGHT;

            bool onSnake = false;
            for (int sid : gSnakeSegments) {
                Entity* s = findEntity(sid);
                if (s && s->gx == x && s->gy == y) {
                    onSnake = true;
                    break;
                }
            }
            if (!onSnake) {
                food.gx = x;
                food.gy = y;
                return;
            }
            if (++attempts > 100) { // por si acaso, evita bucle infinito
                food.gx = x;
                food.gy = y;
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
        placeFoodRandom(f);
        gEntities.push_back(f);
        std::cout << "[Engine] ensureFoodExists -> Food id=" << f.id
                  << " at (" << f.gx << "," << f.gy << ")\n";
    }

    // ---------------------------------------------------------------------
    // TETRIS: helper para crear una nueva pieza
    // ---------------------------------------------------------------------

    static int spawnRandomTetrisPiece() {
        // Tipos simples (1 bloque) pero diferentes nombres/colores
        const std::string shapes[] = { "I", "O", "T", "L", "Z" };
        int idx = std::rand() % 5;
        std::string t = shapes[idx];

        Entity e;
        e.id   = gNextId++;
        e.w    = 1;
        e.h    = 1;
        e.type = t;

        e.gx = BOARD_WIDTH / 2;
        e.gy = 0;

        gEntities.push_back(e);
        gTetrisId = e.id;

        std::cout << "[Engine] spawnRandomTetrisPiece type=" << t
                  << " id=" << e.id << " at (" << e.gx << "," << e.gy << ")\n";

        return e.id;
    }

    static void fixTetrisPiece(Entity* e) {
        if (!e) return;
        e->type = "Fixed";
        gTetrisId = -1;
        std::cout << "[Engine] Tetris piece fixed id=" << e->id
                  << " at (" << e->gx << "," << e->gy << ")\n";

        // Crear una nueva pieza
        spawnRandomTetrisPiece();
    }

    // ---------------------------------------------------------------------
    // Inicialización / apagado
    // ---------------------------------------------------------------------

    void initEngine() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "[Engine] SDL_Init error: " << SDL_GetError() << "\n";
            std::exit(1);
        }

        gWindow = SDL_CreateWindow(
            "Proyecto TLP - Motor",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
        );

        if (!gWindow) {
            std::cerr << "[Engine] SDL_CreateWindow error: " << SDL_GetError() << "\n";
            SDL_Quit();
            std::exit(1);
        }

        gRenderer = SDL_CreateRenderer(
            gWindow,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );

        if (!gRenderer) {
            std::cerr << "[Engine] SDL_CreateRenderer error: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(gWindow);
            SDL_Quit();
            std::exit(1);
        }

        std::srand(static_cast<unsigned>(std::time(nullptr)));

        gEntities.clear();
        gSnakeSegments.clear();
        gNextId    = 1;
        gScore     = 0;
        gGameEnded = false;
        gTetrisId  = -1;
        gSnakeId   = -1;
        gSnakeDirX = 1;
        gSnakeDirY = 0;

        std::cout << "[Engine] initEngine() - SDL initialized\n";
    }

    void shutdownEngine() {
        std::cout << "[Engine] shutdownEngine()\n";
        if (gRenderer) {
            SDL_DestroyRenderer(gRenderer);
            gRenderer = nullptr;
        }
        if (gWindow) {
            SDL_DestroyWindow(gWindow);
            gWindow = nullptr;
        }
        SDL_Quit();
    }

    // ---------------------------------------------------------------------
    // Eventos
    // ---------------------------------------------------------------------

    bool pollEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gGameEnded = true;
                return false;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        gGameEnded = true;
                        return false;

                    // --- Controles Tetris: mover pieza con ← y → -----
                    case SDLK_LEFT:
                        if (gTetrisId != -1) {
                            moveEntity(gTetrisId, -1, 0);
                        }
                        break;
                    case SDLK_RIGHT:
                        if (gTetrisId != -1) {
                            moveEntity(gTetrisId,  1, 0);
                        }
                        break;
                    case SDLK_DOWN:
                        if (gTetrisId != -1) {
                            // Caída rápida: intentamos mover 1 hacia abajo
                            moveEntity(gTetrisId, 0, 1);
                        }
                        break;

                    // --- Controles Snake con WASD --------------------
                    case SDLK_w:
                        gSnakeDirX = 0;
                        gSnakeDirY = -1;
                        break;
                    case SDLK_s:
                        gSnakeDirX = 0;
                        gSnakeDirY = 1;
                        break;
                    case SDLK_a:
                        gSnakeDirX = -1;
                        gSnakeDirY = 0;
                        break;
                    case SDLK_d:
                        gSnakeDirX = 1;
                        gSnakeDirY = 0;
                        break;

                    default:
                        break;
                }
            }
        }
        return !gGameEnded;
    }

    // ---------------------------------------------------------------------
    // Dibujo
    // ---------------------------------------------------------------------

    void presentFrame() {
        // Fondo negro
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);

        // Tamaño de celda en píxeles según la grilla lógica
        int tileW = WINDOW_WIDTH  / BOARD_WIDTH;
        int tileH = WINDOW_HEIGHT / BOARD_HEIGHT;

        // Dibujar cada entidad
        for (const auto &e : gEntities) {
            int px = e.gx * tileW;
            int py = e.gy * tileH;
            int pw = e.w  * tileW;
            int ph = e.h  * tileH;

            // Colores por tipo
            if (e.type == "I" || e.type == "Fixed") {
                SDL_SetRenderDrawColor(gRenderer, 0, 200, 255, 255);     // Azul
            } else if (e.type == "O") {
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255);     // Amarillo
            } else if (e.type == "T") {
                SDL_SetRenderDrawColor(gRenderer, 200, 0, 200, 255);     // Morado
            } else if (e.type == "L") {
                SDL_SetRenderDrawColor(gRenderer, 255, 165, 0, 255);     // Naranja
            } else if (e.type == "Z") {
                SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);       // Rojo
            } else if (isSnakeAnyType(e.type)) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);       // Snake
            } else if (isFoodType(e.type)) {
                SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);       // Fruta
            } else {
                SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);   // Otros
            }

            SDL_Rect r{ px, py, pw, ph };
            SDL_RenderFillRect(gRenderer, &r);
        }

        SDL_RenderPresent(gRenderer);
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
        std::cout << "[Engine] addScore " << delta << " => " << gScore << "\n";
    }

    // ---------------------------------------------------------------------
    // Spawn de bloques / entidades (desde el script)
    // ---------------------------------------------------------------------

    int spawnBlock(const std::string& typeIn, int gridX, int gridY) {
        std::string t = typeIn;

        // 1) Si el script pide una pieza Tetris y no hay activa,
        //    ignoramos el tipo concreto y creamos una aleatoria.
        if (isTetrisType(t)) {
            if (gTetrisId == -1) {
                return spawnRandomTetrisPiece();
            } else {
                // Ya hay una pieza activa, no creamos otra. Devolvemos la existente.
                std::cout << "[Engine] spawnBlock(Tetris) called but piece already active id="
                          << gTetrisId << "\n";
                return gTetrisId;
            }
        }

        // 2) Si no hay serpiente aún, este bloque será la cabeza
        if (gSnakeId == -1) {
            Entity e;
            e.id   = gNextId++;
            e.gx   = gridX;
            e.gy   = gridY;
            e.w    = 1;
            e.h    = 1;
            e.type = "Snake";

            gSnakeId  = e.id;
            gSnakeDirX = 1;
            gSnakeDirY = 0;
            gSnakeSegments.clear();
            gSnakeSegments.push_back(e.id);

            gEntities.push_back(e);

            std::cout << "[Engine] spawnBlock -> Snake head id=" << e.id
                      << " at (" << e.gx << "," << e.gy << ")\n";
            return e.id;
        }

        // 3) Si ya hay serpiente, consideramos la entidad como fruta
        Entity f;
        f.id   = gNextId++;
        f.w    = 1;
        f.h    = 1;
        f.type = "Food";
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

        // --- Piezas ya fijadas de Tetris: no se mueven -------------------
        if (e->type == "Fixed") {
            return;
        }

        // -----------------------------------------------------------------
        // TETRIS
        // -----------------------------------------------------------------
        if (isTetrisType(e->type)) {
            int newGx = e->gx + dx;
            int newGy = e->gy + dy;
            int bottomY = BOARD_HEIGHT - e->h;

            // Limitar bordes laterales
            if (newGx < 0) newGx = 0;
            if (newGx > BOARD_WIDTH - e->w)
                newGx = BOARD_WIDTH - e->w;

            // ¿Colisión con fondo?
            if (newGy >= bottomY) {
                e->gx = newGx;
                e->gy = bottomY;
                fixTetrisPiece(e);
                return;
            }

            // ¿Colisión con bloque Fixed justo debajo?
            bool collide = false;
            for (const auto &f : gEntities) {
                if (f.type == "Fixed" &&
                    f.gx == newGx && f.gy == newGy) {
                    collide = true;
                    break;
                }
            }

            if (collide) {
                // Se fija en la posición anterior
                fixTetrisPiece(e);
                return;
            }

            // Movimiento normal
            e->gx = newGx;
            e->gy = newGy;

            std::cout << "[Engine] moveEntity(Tetris) id=" << id
                      << " dx=" << dx << " dy=" << dy
                      << " => (" << e->gx << "," << e->gy << ")\n";
            return;
        }

        // -----------------------------------------------------------------
        // SNAKE (cabeza)
        // -----------------------------------------------------------------
        if (isSnakeHeadType(e->type)) {

            // Aseguramos que exista al menos una fruta
            ensureFoodExists();

            // Posiciones antiguas de TODOS los segmentos
            std::vector<std::pair<int,int>> oldPos;
            oldPos.reserve(gSnakeSegments.size());
            for (int sid : gSnakeSegments) {
                Entity* s = findEntity(sid);
                if (s) oldPos.emplace_back(s->gx, s->gy);
                else   oldPos.emplace_back(0, 0);
            }

            // Nueva posición de la cabeza usando SIEMPRE la dirección actual
            int newHeadX = e->gx + gSnakeDirX;
            int newHeadY = e->gy + gSnakeDirY;

            // wrap-around para Snake
            if (newHeadX < 0) newHeadX = BOARD_WIDTH - 1;
            if (newHeadX >= BOARD_WIDTH) newHeadX = 0;
            if (newHeadY < 0) newHeadY = BOARD_HEIGHT - 1;
            if (newHeadY >= BOARD_HEIGHT) newHeadY = 0;

            // ¿Va a comer fruta?
            bool willEat = false;
            Entity* eatenFood = nullptr;
            for (auto &f : gEntities) {
                if (isFoodType(f.type) && f.gx == newHeadX && f.gy == newHeadY) {
                    willEat = true;
                    eatenFood = &f;
                    break;
                }
            }

            // Comprobamos colisión con el cuerpo (ignorando cola si no crece)
            for (std::size_t i = 0; i < gSnakeSegments.size(); ++i) {
                if (!willEat && i == gSnakeSegments.size() - 1) {
                    continue; // cola se libera
                }
                Entity* s = findEntity(gSnakeSegments[i]);
                if (s && s->gx == newHeadX && s->gy == newHeadY) {
                    endGame("Snake: self collision");
                    return;
                }
            }

            // Si come, sumamos puntos y recolocamos esa fruta
            if (willEat && eatenFood) {
                addScore(10);
                placeFoodRandom(*eatenFood);
                std::cout << "[Engine] Snake ate food -> new food at ("
                          << eatenFood->gx << "," << eatenFood->gy << ")\n";
            }

            // Actualizar cuerpo
            if (!gSnakeSegments.empty()) {
                // cabeza
                e->gx = newHeadX;
                e->gy = newHeadY;

                // resto de segmentos
                for (std::size_t i = 1; i < gSnakeSegments.size(); ++i) {
                    Entity* seg = findEntity(gSnakeSegments[i]);
                    if (seg && i-1 < oldPos.size()) {
                        seg->gx = oldPos[i-1].first;
                        seg->gy = oldPos[i-1].second;
                    }
                }

                // Si comió, nuevo segmento en antigua cola
                if (willEat && !oldPos.empty()) {
                    Entity tailSeg;
                    tailSeg.id   = gNextId++;
                    tailSeg.w    = 1;
                    tailSeg.h    = 1;
                    tailSeg.type = "SnakeBody";
                    tailSeg.gx   = oldPos.back().first;
                    tailSeg.gy   = oldPos.back().second;
                    gEntities.push_back(tailSeg);
                    gSnakeSegments.push_back(tailSeg.id);

                    std::cout << "[Engine] Snake grew -> new segment id="
                              << tailSeg.id << " at ("
                              << tailSeg.gx << "," << tailSeg.gy << ")\n";
                }
            }

            std::cout << "[Engine] moveEntity(Snake) id=" << id
                      << " => (" << e->gx << "," << e->gy << ")\n";
            return;
        }

        // -----------------------------------------------------------------
        // Otros tipos genéricos
        // -----------------------------------------------------------------
        int newGx = e->gx + dx;
        int newGy = e->gy + dy;

        if (newGx < 0) newGx = 0;
        if (newGx > BOARD_WIDTH - e->w)
            newGx = BOARD_WIDTH - e->w;
        if (newGy < 0) newGy = 0;
        if (newGy > BOARD_HEIGHT - e->h)
            newGy = BOARD_HEIGHT - e->h;

        e->gx = newGx;
        e->gy = newGy;

        std::cout << "[Engine] moveEntity id=" << id
                  << " dx=" << dx << " dy=" << dy
                  << " => (" << e->gx << "," << e->gy << ")\n";
    }

    // ---------------------------------------------------------------------
    // Estado del juego
    // ---------------------------------------------------------------------

    bool isGameEnded() {
        return gGameEnded;
    }

    // ---------------------------------------------------------------------
    // STUBS y utilidades
    // ---------------------------------------------------------------------

    void rotateEntity(int id) {
        std::cout << "[Engine] rotateEntity id=" << id << " (stub)\n";
    }

    void dropEntity(int id) {
        Entity* e = findEntity(id);
        if (!e) return;

        while (e->gy < BOARD_HEIGHT - e->h) {
            e->gy += 1;
        }
        std::cout << "[Engine] dropEntity id=" << id
                  << " -> bottom\n";
    }

    void endGame(const std::string& r) {
        gGameEnded = true;
        std::cout << "[Engine] endGame() called. Reason: " << r << "\n";
    }

    void drawText(const std::string& t, int x, int y) {
        std::cout << "[Engine] drawText \"" << t << "\" at ("
                  << x << "," << y << ") (stub, no rendering)\n";
    }

} // namespace Engine
