#include "interpreter/script_interpreter.h"
#include "engine/api.h"

#include <iostream>
#include <cstdlib>
#include <string>

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>

// ---------------------------------------------------------------------
// Bucle principal que corre el AST
// ---------------------------------------------------------------------
static int runGame(const std::string& ast_path,
                   int frames = 1000000,
                   int ms_per_frame = 120)
{
    std::cout << "[XP] runGame() iniciado, AST: " << ast_path << "\n";

    std::cout << "[XP] Inicializando motor...\n";
    Engine::initEngine();
    std::cout << "[XP] Motor inicializado OK.\n";

    std::cout << "[XP] Creando ScriptInterpreter...\n";
    ScriptInterpreter interp;

    std::cout << "[XP] Cargando AST...\n";
    if (!interp.loadASTFile(ast_path)) {
        std::cerr << "[XP] ERROR: Fallo cargando AST: " << ast_path << "\n";
        Engine::shutdownEngine();
        return 1;
    }
    std::cout << "[XP] AST cargado OK.\n";

    const std::string className = "Game";

    std::cout << "[XP] Llamando Game.init()...\n";
    interp.callMethod(className, "init");
    std::cout << "[XP] Game.init() terminado.\n";

    int f = 0;
    std::cout << "[XP] Entrando al bucle principal...\n";
    while (f < frames && Engine::pollEvents() && !Engine::isGameEnded()) {
        interp.callMethod(className, "update");
        Engine::presentFrame();
        SDL_Delay(ms_per_frame);
        ++f;

        if ((f % 1000) == 0) {
            std::cout << "[XP] Frame " << f << "\n";
        }
    }

    std::cout << "[XP] Bucle principal terminado. frames=" << f << "\n";

    if (!Engine::isGameEnded()) {
        std::cout << "[XP] Llamando Game.end()...\n";
        interp.callMethod(className, "end");
        std::cout << "[XP] Game.end() terminado.\n";
    }

    std::cout << "[XP] Apagando motor...\n";
    Engine::shutdownEngine();
    std::cout << "[XP] runGame() termina OK.\n";

    return 0;
}

// ---------------------------------------------------------------------
// main "normal" (se usa en Wine y cuando se llama desde consola)
// ---------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::cout << "[XP] main() arrancando. argc=" << argc << "\n";

    if (argc >= 2) {
        std::string ast_path = argv[1];
        int frames = 1000000;
        int ms_per_frame = 120;

        if (argc >= 3) {
            frames = std::atoi(argv[2]);
            if (frames <= 0) frames = 1000000;
        }

        if (argc >= 4) {
            ms_per_frame = std::atoi(argv[3]);
            if (ms_per_frame <= 0) ms_per_frame = 120;
        }

        std::cout << "[XP] Modo CLI. AST=" << ast_path
                  << " frames=" << frames
                  << " ms=" << ms_per_frame << "\n";

        return runGame(ast_path, frames, ms_per_frame);
    }

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
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            opcion = 0;
        }
    }

    std::string ast_path;
    if (opcion == 1) {
        ast_path = "games/tetris.ast.json";
    } else {
        ast_path = "games/snake.ast.json";
    }

    std::cout << "[XP] Opción seleccionada: " << opcion
              << " AST=" << ast_path << "\n";

    return runGame(ast_path);
}

#ifdef _WIN32
// ---------------------------------------------------------------------
// WinMain para contentar al CRT de MinGW cuando pide WinMain@16
// ---------------------------------------------------------------------
#include <windows.h>

extern "C"
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // Para doble click en XP: no hay argumentos, no nos importa.
    return main(0, 0);
}
#endif
