#include "script_interpreter.h"
#include "../engine/api.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

ScriptInterpreter::ScriptInterpreter() {}

bool ScriptInterpreter::loadASTFile(const std::string &path) {
    std::ifstream f(path);
    if(!f.is_open()) {
        std::cerr << "No se pudo abrir " << path << "\n";
        return false;
    }
    json ast;
    try {
        f >> ast;
    } catch(std::exception &e) {
        std::cerr << "Error parseando JSON: " << e.what() << "\n";
        return false;
    }

    // Esperamos un AST con root.children array
    if(!ast.contains("children") || !ast["children"].is_array()) {
        std::cerr << "AST no tiene 'children' como array\n";
        return false;
    }

    for(auto &node : ast["children"]) {
        if(!node.contains("node")) continue;
        std::string nodename = node["node"].get<std::string>();
        if(nodename == "Class") {
            GameClass gc;
            if(node.contains("props") && node["props"].contains("name"))
                gc.name = node["props"]["name"].get<std::string>();
            // buscar metodos
            if(node.contains("children") && node["children"].is_array()) {
                for(auto &child : node["children"]) {
                    if(!child.contains("node")) continue;
                    std::string cn = child["node"].get<std::string>();
                    if(cn == "Method") {
                        Method m;
                        if(child.contains("props") && child["props"].contains("name"))
                            m.name = child["props"]["name"].get<std::string>();
                        if(child.contains("children")) m.body = child["children"];
                        gc.methods[m.name] = m;
                    }
                }
            }
            classes[gc.name] = gc;
        }
    }
    std::cout << "AST cargado. Clases encontradas: " << classes.size() << "\n";
    for(auto &p: classes) std::cout<<" - "<<p.first<<"\n";
    return true;
}

void ScriptInterpreter::executeCall(const json &callNode) {
    // Espera: { "node":"Call", "props": {"name":"spawnBlock", "args":[...] } }
    if(!callNode.contains("props") || !callNode["props"].contains("name")) return;
    std::string name = callNode["props"]["name"].get<std::string>();
    auto args = callNode["props"].value("args", json::array());

    if(name == "spawnBlock") {
        std::string type = args.size() > 0 ? args[0].get<std::string>() : "default";
        int x = args.size() > 1 ? args[1].get<int>() : 0;
        int y = args.size() > 2 ? args[2].get<int>() : 0;
        Engine::spawnBlock(type, x, y);
    } else if(name == "moveEntity") {
        int id = args.size()>0 ? args[0].get<int>() : 0;
        int dx = args.size()>1 ? args[1].get<int>() : 0;
        int dy = args.size()>2 ? args[2].get<int>() : 0;
        Engine::moveEntity(id, dx, dy);
    } else if(name == "rotateEntity") {
        int id = args.size()>0 ? args[0].get<int>() : 0;
        Engine::rotateEntity(id);
    } else if(name == "dropEntity") {
        int id = args.size()>0 ? args[0].get<int>() : 0;
        Engine::dropEntity(id);
    } else if(name == "addScore") {
        // COMENTADO: El puntaje se maneja internamente en api.cpp
        // int s = args.size()>0 ? args[0].get<int>() : 0;
        // Engine::addScore(s);
        std::cout << "[Interpreter] addScore ignorado (manejado internamente por el motor)\n";
    } else if(name == "setScore") {
        int s = args.size()>0 ? args[0].get<int>() : 0;
        Engine::setScore(s);
    } else if(name == "endGame") {
        std::string r = args.size()>0 ? args[0].get<std::string>() : "unknown";
        Engine::endGame(r);
    } else if(name == "drawText") {
        std::string t = args.size()>0 ? args[0].get<std::string>() : "";
        int x = args.size()>1 ? args[1].get<int>() : 0;
        int y = args.size()>2 ? args[2].get<int>() : 0;
        Engine::drawText(t,x,y);
    } else {
        std::cout << "[Interpreter] Llamada desconocida: " << name << "\n";
    }
}

void ScriptInterpreter::executeStatement(const json &stmt) {
    if(!stmt.contains("node")) return;
    std::string n = stmt["node"].get<std::string>();
    if(n == "Call") {
        executeCall(stmt);
    } else {
        // puedes ampliar más tipos (Assign, If, While...)
        std::cout << "[Interpreter] Statement no manejado: " << n << "\n";
    }
}

void ScriptInterpreter::callMethod(const std::string &className, const std::string &methodName) {
    auto it = classes.find(className);
    if(it == classes.end()) {
        std::cerr << "Clase " << className << " no encontrada\n";
        return;
    }
    auto &gc = it->second;
    auto mit = gc.methods.find(methodName);
    if(mit == gc.methods.end()) {
        std::cerr << "Metodo " << methodName << " en clase " << className << " no encontrado\n";
        return;
    }
    auto &body = mit->second.body;
    if(!body.is_array()) return;
    for(auto &stmt : body) {
        executeStatement(stmt);
    }
}

void ScriptInterpreter::runLoop(const std::string &className, const std::string &updateMethodName, int frames, int ms_per_frame) {
    // llamar init si existe
    callMethod(className, "init");
    for(int f=0; f<frames; ++f) {
        callMethod(className, updateMethodName);
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_per_frame));
    }
    // llamar end si existe
    callMethod(className, "end");
}
