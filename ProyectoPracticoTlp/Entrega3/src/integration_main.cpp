#include "interpreter/script_interpreter.h"
#include "engine/api.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <SDL.h>

// Pequeña función que corre un juego dado el path del AST
static int runGame(const std::string& ast_path,
                   int frames = 1'000'000,
                   int ms_per_frame = 120)   // un poco más lento para XP
{
    // Inicializar motor gráfico
    Engine::initEngine();

    // Cargar intérprete y AST
    ScriptInterpreter interp;
    if (!interp.loadASTFile(ast_path)) {
        std::cerr << "Fallo cargando AST: " << ast_path << "\n";
        Engine::shutdownEngine();
        return 1;
    }

    const std::string className = "Game";

    // Llamar init()
    interp.callMethod(className, "init");

    int f = 0;
    while (f < frames && Engine::pollEvents() && !Engine::isGameEnded()) {
        // Llamar update() definido en el AST
        interp.callMethod(className, "update");

        // Dibujar frame con SDL
        Engine::presentFrame();

        // Pequeña pausa entre frames
        SDL_Delay(ms_per_frame);
        ++f;
    }

    // Si el motor no marcó fin de juego, llamamos end() del script
    if (!Engine::isGameEnded()) {
        interp.callMethod(className, "end");
    }

    // Apagar motor gráfico
    Engine::shutdownEngine();
    return 0;
}

int main(int argc, char** argv)
{
    // ---------------------------------------------------------------------
    // MODO 1: como antes, pasando el AST por parámetro
    // ---------------------------------------------------------------------
    if (argc >= 2) {
        std::string ast_path = argv[1];
        int frames = 1'000'000;
        int ms_per_frame = 120;

        if (argc >= 3) frames       = std::atoi(argv[2]);
        if (argc >= 4) ms_per_frame = std::atoi(argv[3]);

        return runGame(ast_path, frames, ms_per_frame);
    }

    // ---------------------------------------------------------------------
    // MODO 2: menú en consola (sin argumentos)
    // ---------------------------------------------------------------------
    std::cout << "=====================================\n";
    std::cout << "   Proyecto Practico TLP - Entrega 3\n";
    std::cout << "=====================================\n\n";
    std::cout << "Seleccione el juego a ejecutar:\n";
    std::cout << "  1) Tetris\n";
    std::cout << "  2) Snake\n\n";

    int opcion = 0;
    while (opcion != 1 && opcion != 2) {
        std::cout << "Digite 1 o 2 y presione ENTER: ";
        if (!(std::cin >> opcion)) {
            // Si hay error de entrada, limpiamos y volvemos a pedir
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            opcion = 0;
        }
    }

    std::string ast_path;
    switch (opcion) {
        case 1:
            ast_path = "games/tetris.ast.json";
            break;
        case 2:
            ast_path = "games/snake.ast.json";
            break;
    }

    // Llamamos al motor con el AST seleccionado
    return runGame(ast_path);
}
