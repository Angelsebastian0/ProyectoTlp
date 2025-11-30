#pragma once

#include <string>

namespace Engine {

    // Parámetros básicos del tablero
    constexpr int TILE_SIZE    = 32;
    constexpr int BOARD_WIDTH  = 10;   // 10 columnas
    constexpr int BOARD_HEIGHT = 20;   // 20 filas

    // Pequeño vector 2D que usaba el intérprete
    struct Vec2 {
        int x;
        int y;
    };

    // Inicialización / apagado del motor
    void initEngine();
    void shutdownEngine();

    // Loop principal
    bool pollEvents();      // Procesa eventos SDL (cierra con X o ESC)
    void presentFrame();    // Dibuja todo en pantalla

    // API principal que usa ahora el motor
    int  spawnBlock(const std::string& type, int gridX, int gridY);
    void moveEntity(int id, int dx, int dy);
    void setScore(int value);
    void addScore(int delta);
    bool isGameEnded();

    // --- WRAPPERS para mantener compatibilidad con el intérprete --------
    // El intérprete llama a estas versiones con Vec2, las redirigimos
    inline int spawnBlock(const std::string& type, const Vec2& pos) {
        return spawnBlock(type, pos.x, pos.y);
    }

    inline void moveEntity(int id, const Vec2& d) {
        moveEntity(id, d.x, d.y);
    }

    // El intérprete también llama a estas; las implementamos como stubs
    void rotateEntity(int id);
    void dropEntity(int id);
    void endGame(const std::string& r);
    void drawText(const std::string& t, int x, int y);
}
