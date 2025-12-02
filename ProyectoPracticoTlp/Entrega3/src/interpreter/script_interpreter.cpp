#include "script_interpreter.h"
#include "../engine/api.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

ScriptInterpreter::ScriptInterpreter() {}

static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool ScriptInterpreter::loadASTFile(const std::string &path) {
    std::ifstream f(path.c_str());
    if(!f.is_open()) {
        std::cerr << "No se pudo abrir " << path << "\n";
        return false;
    }

    methods.clear();

    std::string line;
    Method current;
    bool hasCurrent = false;

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line[line.size()-1] == ']') {
            if (hasCurrent) {
                methods[current.name] = current;
            }
            current = Method();
            current.name = line.substr(1, line.size() - 2);
            current.commands.clear();
            hasCurrent = true;
            continue;
        }

        std::istringstream iss(line);
        Command cmd;
        if (!(iss >> cmd.name)) continue;
        std::string token;
        while (iss >> token) {
            cmd.args.push_back(token);
        }
        current.commands.push_back(cmd);
    }

    if (hasCurrent) {
        methods[current.name] = current;
    }

    std::cout << "[Interpreter] Script cargado. Metodos: " << methods.size() << "\n";
    return !methods.empty();
}

void ScriptInterpreter::executeCommand(const Command &cmd) {
    if (cmd.name == "spawnBlock") {
        std::string type = cmd.args.size() > 0 ? cmd.args[0] : "";
        int x = cmd.args.size() > 1 ? std::atoi(cmd.args[1].c_str()) : 0;
        int y = cmd.args.size() > 2 ? std::atoi(cmd.args[2].c_str()) : 0;
        Engine::spawnBlock(type, x, y);
    } else if (cmd.name == "moveEntity") {
        int id = cmd.args.size() > 0 ? std::atoi(cmd.args[0].c_str()) : 0;
        int dx = cmd.args.size() > 1 ? std::atoi(cmd.args[1].c_str()) : 0;
        int dy = cmd.args.size() > 2 ? std::atoi(cmd.args[2].c_str()) : 0;
        Engine::moveEntity(id, dx, dy);
    } else if (cmd.name == "rotateEntity") {
        int id = cmd.args.size() > 0 ? std::atoi(cmd.args[0].c_str()) : 0;
        Engine::rotateEntity(id);
    } else if (cmd.name == "dropEntity") {
        int id = cmd.args.size() > 0 ? std::atoi(cmd.args[0].c_str()) : 0;
        Engine::dropEntity(id);
    } else if (cmd.name == "addScore") {
        int delta = cmd.args.size() > 0 ? std::atoi(cmd.args[0].c_str()) : 0;
        Engine::addScore(delta);
    } else if (cmd.name == "setScore") {
        int v = cmd.args.size() > 0 ? std::atoi(cmd.args[0].c_str()) : 0;
        Engine::setScore(v);
    } else if (cmd.name == "endGame") {
        std::string reason = cmd.args.size() > 0 ? cmd.args[0] : "";
        Engine::endGame(reason);
    } else if (cmd.name == "drawText") {
        std::string text = cmd.args.size() > 0 ? cmd.args[0] : "";
        int x = cmd.args.size() > 1 ? std::atoi(cmd.args[1].c_str()) : 0;
        int y = cmd.args.size() > 2 ? std::atoi(cmd.args[2].c_str()) : 0;
        Engine::drawText(text, x, y);
    } else {
        std::cout << "[Interpreter] Comando desconocido: " << cmd.name << "\n";
    }
}

void ScriptInterpreter::callMethod(const std::string &className, const std::string &methodName) {
    (void)className; // mantenemos firma, pero no usamos clases
    std::map<std::string, Method>::iterator it = methods.find(methodName);
    if (it == methods.end()) {
        std::cerr << "Metodo " << methodName << " no encontrado\n";
        return;
    }

    const std::vector<Command> &body = it->second.commands;
    for (size_t i = 0; i < body.size(); ++i) {
        executeCommand(body[i]);
    }
}

void ScriptInterpreter::runLoop(const std::string &className, const std::string &updateMethodName, int frames, int ms_per_frame) {
    callMethod(className, "init");

    for (int f = 0; f < frames && !Engine::isGameEnded(); ++f) {
        Engine::presentFrame();
        callMethod(className, updateMethodName);
#ifdef _WIN32
        Sleep(ms_per_frame);
#else
        usleep(static_cast<useconds_t>(ms_per_frame) * 1000);
#endif
        if (!Engine::pollEvents()) break;
    }

    if (!Engine::isGameEnded()) {
        callMethod(className, "end");
    }
}
