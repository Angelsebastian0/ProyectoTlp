#include "interpreter/script_interpreter.h"
#include "engine/api.h"

#include <iostream>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static void sleepMs(int ms) {
#ifdef _WIN32
    Sleep(static_cast<DWORD>(ms));
#else
    usleep(static_cast<useconds_t>(ms) * 1000);
#endif
}

static int runGame(const std::string& script_path,
                   int frames,
                   int ms_per_frame)
{
    Engine::initEngine();

    ScriptInterpreter interp;
    if (!interp.loadASTFile(script_path)) {
        std::cerr << "Fallo cargando script: " << script_path << "\n";
        Engine::shutdownEngine();
        return 1;
    }

    const std::string className = "Game";

    interp.callMethod(className, "init");

    int f = 0;
    while (f < frames && Engine::pollEvents() && !Engine::isGameEnded()) {
        interp.callMethod(className, "update");
        Engine::presentFrame();
        sleepMs(ms_per_frame);
        ++f;
    }

    if (!Engine::isGameEnded()) {
        interp.callMethod(className, "end");
    }

    Engine::shutdownEngine();
    return 0;
}

int main(int argc, char** argv)
{
    if (argc >= 2) {
        std::string script_path = argv[1];
        int frames = 1000000;
        int ms_per_frame = 120;

        if (argc >= 3) frames       = std::atoi(argv[2]);
        if (argc >= 4) ms_per_frame = std::atoi(argv[3]);

        return runGame(script_path, frames, ms_per_frame);
    }

    std::cout << "=====================================\n";
    std::cout << "   Proyecto Practico TLP - Entrega 3\n";
    std::cout << "   Modo consola (sin SDL/JSON)\n";
    std::cout << "=====================================\n\n";
    std::cout << "Seleccione el juego a ejecutar:\n";
    std::cout << "  1) Tetris (demostracion basica)\n";
    std::cout << "  2) Snake (demostracion basica)\n\n";

    int opcion = 0;
    while (opcion != 1 && opcion != 2) {
        std::cout << "Digite 1 o 2 y presione ENTER: ";
        if (!(std::cin >> opcion)) {
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            opcion = 0;
        }
    }

    std::string script_path;
    switch (opcion) {
        case 1:
            script_path = "games/tetris.script";
            break;
        case 2:
            script_path = "games/snake.script";
            break;
        default:
            script_path = "games/snake.script";
            break;
    }

    return runGame(script_path, 1000000, 120);
}
