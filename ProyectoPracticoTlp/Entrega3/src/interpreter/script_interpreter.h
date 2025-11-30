#ifndef SCRIPT_INTERPRETER_H
#define SCRIPT_INTERPRETER_H

#include "json.hpp"
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

struct Method {
    std::string name;
    json body; // array of statements
};

struct GameClass {
    std::string name;
    std::unordered_map<std::string, Method> methods;
};

class ScriptInterpreter {
public:
    ScriptInterpreter();
    bool loadASTFile(const std::string &path);
    void callMethod(const std::string &className, const std::string &methodName);
    void runLoop(const std::string &className, const std::string &updateMethodName = "update", int frames = 200, int ms_per_frame = 16);
private:
    std::unordered_map<std::string, GameClass> classes;
    void executeStatement(const json &stmt);
    void executeCall(const json &callNode);
};

#endif // SCRIPT_INTERPRETER_H
