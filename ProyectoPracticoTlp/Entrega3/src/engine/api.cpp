#include "engine/api.h"

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#endif

namespace Engine {

    struct Entity {
        int id;
        int gx;
        int gy;
        int w;
        int h;
        std::string type;
    };

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

#ifdef _WIN32
#else
    static bool gTermConfigured = false;

    static void configureTerminal() {
        if (gTermConfigured) return;
        gTermConfigured = true;
        termios t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }

    static int _kbhit() {
        configureTerminal();
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    }

    static int _getch() {
        configureTerminal();
        return getchar();
    }
#endif

    // ---------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------

    static Entity* findEntity(int id) {
        for (size_t i = 0; i < gEntities.size(); ++i) {
            if (gEntities[i].id == id) return &gEntities[i];
        }
        return NULL;
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

    static bool isFoodType(const std::string& t) {
        return (t == "Food");
    }

    static char symbolFor(const std::string& t) {
        if (t == "I" || t == "Block" || t == "Fixed") return '#';
        if (t == "O") return 'O';
        if (t == "T") return 'T';
        if (t == "L") return 'L';
        if (t == "Z") return 'Z';
        if (isSnakeHeadType(t)) return 'S';
        if (isSnakeBodyType(t)) return 's';
        if (isFoodType(t)) return 'F';
        return '?';
    }

    static bool hasFood() {
        for (size_t i = 0; i < gEntities.size(); ++i) {
            if (isFoodType(gEntities[i].type)) return true;
        }
        return false;
    }

    static void placeFoodRandom(Entity &food) {
        int attempts = 0;
        while (true) {
            int x = std::rand() % BOARD_WIDTH;
            int y = std::rand() % BOARD_HEIGHT;

            bool onSnake = false;
            for (size_t i = 0; i < gSnakeSegments.size(); ++i) {
                Entity* s = findEntity(gSnakeSegments[i]);
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
            ++attempts;
            if (attempts > 100) {
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

    static int spawnRandomTetrisPiece() {
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

        spawnRandomTetrisPiece();
    }

    // ---------------------------------------------------------------------
    // Inicialización / apagado
    // ---------------------------------------------------------------------

    void initEngine() {
        std::srand(static_cast<unsigned>(std::time(NULL)));

        gEntities.clear();
        gSnakeSegments.clear();
        gNextId    = 1;
        gScore     = 0;
        gGameEnded = false;
        gTetrisId  = -1;
        gSnakeId   = -1;
        gSnakeDirX = 1;
        gSnakeDirY = 0;

        std::cout << "[Engine] initEngine() - modo consola\n";
    }

    void shutdownEngine() {
        std::cout << "[Engine] shutdownEngine()\n";
    }

    // ---------------------------------------------------------------------
    // Eventos
    // ---------------------------------------------------------------------

    bool pollEvents() {
        if (gGameEnded) return false;

        if (_kbhit()) {
            int key = _getch();
            switch (key) {
                case 'q':
                case 'Q':
                case 27: // ESC
                    gGameEnded = true;
                    return false;
                // Controles Tetris
                case 'j':
                    if (gTetrisId != -1) moveEntity(gTetrisId, -1, 0);
                    break;
                case 'l':
                    if (gTetrisId != -1) moveEntity(gTetrisId, 1, 0);
                    break;
                case 'k':
                    if (gTetrisId != -1) moveEntity(gTetrisId, 0, 1);
                    break;
                // Controles Snake
                case 'w':
                case 'W':
                    gSnakeDirX = 0; gSnakeDirY = -1; break;
                case 's':
                case 'S':
                    gSnakeDirX = 0; gSnakeDirY = 1; break;
                case 'a':
                case 'A':
                    gSnakeDirX = -1; gSnakeDirY = 0; break;
                case 'd':
                case 'D':
                    gSnakeDirX = 1; gSnakeDirY = 0; break;
                default:
                    break;
            }
        }

        return !gGameEnded;
    }

    // ---------------------------------------------------------------------
    // Dibujo
    // ---------------------------------------------------------------------

    void presentFrame() {
        std::vector<std::string> grid;
        grid.assign(BOARD_HEIGHT, std::string(BOARD_WIDTH, '.'));

        for (size_t i = 0; i < gEntities.size(); ++i) {
            const Entity &e = gEntities[i];
            for (int dy = 0; dy < e.h; ++dy) {
                for (int dx = 0; dx < e.w; ++dx) {
                    int px = e.gx + dx;
                    int py = e.gy + dy;
                    if (px >= 0 && px < BOARD_WIDTH && py >= 0 && py < BOARD_HEIGHT) {
                        grid[py][px] = symbolFor(e.type);
                    }
                }
            }
        }

        std::cout << "\x1b[2J\x1b[H"; // intento de limpiar (ANSI)
        std::cout << "Score: " << gScore << "\n";
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            std::cout << "|" << grid[y] << "|\n";
        }
        std::cout << "Controles: q=salir, wasd=Snake, j/l/k=Tetris" << std::endl;
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

        if (isTetrisType(t)) {
            if (gTetrisId == -1) {
                return spawnRandomTetrisPiece();
            } else {
                std::cout << "[Engine] spawnBlock(Tetris) called but piece already active id="
                          << gTetrisId << "\n";
                return gTetrisId;
            }
        }

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

        if (e->type == "Fixed") {
            return;
        }

        if (isTetrisType(e->type)) {
            int newGx = e->gx + dx;
            int newGy = e->gy + dy;
            int bottomY = BOARD_HEIGHT - e->h;

            if (newGx < 0) newGx = 0;
            if (newGx > BOARD_WIDTH - e->w) newGx = BOARD_WIDTH - e->w;

            if (newGy >= bottomY) {
                e->gx = newGx;
                e->gy = bottomY;
                fixTetrisPiece(e);
                return;
            }

            bool collide = false;
            for (size_t i = 0; i < gEntities.size(); ++i) {
                const Entity &f = gEntities[i];
                if (f.type == "Fixed" && f.gx == newGx && f.gy == newGy) {
                    collide = true;
                    break;
                }
            }

            if (collide) {
                fixTetrisPiece(e);
                return;
            }

            e->gx = newGx;
            e->gy = newGy;

            std::cout << "[Engine] moveEntity(Tetris) id=" << id
                      << " dx=" << dx << " dy=" << dy
                      << " => (" << e->gx << "," << e->gy << ")\n";
            return;
        }

        if (isSnakeHeadType(e->type)) {
            ensureFoodExists();

            std::vector< std::pair<int,int> > oldPos;
            oldPos.reserve(gSnakeSegments.size());
            for (size_t i = 0; i < gSnakeSegments.size(); ++i) {
                Entity* s = findEntity(gSnakeSegments[i]);
                if (s) oldPos.push_back(std::make_pair(s->gx, s->gy));
                else   oldPos.push_back(std::make_pair(0, 0));
            }

            int newHeadX = e->gx + gSnakeDirX;
            int newHeadY = e->gy + gSnakeDirY;

            if (newHeadX < 0) newHeadX = BOARD_WIDTH - 1;
            if (newHeadX >= BOARD_WIDTH) newHeadX = 0;
            if (newHeadY < 0) newHeadY = BOARD_HEIGHT - 1;
            if (newHeadY >= BOARD_HEIGHT) newHeadY = 0;

            bool willEat = false;
            Entity* eatenFood = NULL;
            for (size_t i = 0; i < gEntities.size(); ++i) {
                Entity &f = gEntities[i];
                if (isFoodType(f.type) && f.gx == newHeadX && f.gy == newHeadY) {
                    willEat = true;
                    eatenFood = &f;
                    break;
                }
            }

            for (size_t i = 0; i < gSnakeSegments.size(); ++i) {
                if (!willEat && i == gSnakeSegments.size() - 1) {
                    continue;
                }
                Entity* s = findEntity(gSnakeSegments[i]);
                if (s && s->gx == newHeadX && s->gy == newHeadY) {
                    endGame("Snake: self collision");
                    return;
                }
            }

            if (willEat && eatenFood) {
                addScore(10);
                placeFoodRandom(*eatenFood);
                std::cout << "[Engine] Snake ate food -> new food at ("
                          << eatenFood->gx << "," << eatenFood->gy << ")\n";
            }

            if (!gSnakeSegments.empty()) {
                e->gx = newHeadX;
                e->gy = newHeadY;

                for (size_t i = 1; i < gSnakeSegments.size(); ++i) {
                    Entity* seg = findEntity(gSnakeSegments[i]);
                    if (seg && i-1 < oldPos.size()) {
                        seg->gx = oldPos[i-1].first;
                        seg->gy = oldPos[i-1].second;
                    }
                }

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
                  << x << "," << y << ") (console stub)\n";
    }

} // namespace Engine
