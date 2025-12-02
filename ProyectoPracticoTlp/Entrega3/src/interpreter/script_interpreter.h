#ifndef SCRIPT_INTERPRETER_H
#define SCRIPT_INTERPRETER_H

#include <string>
#include <vector>
#include <map>

struct Command {
    std::string name;
    std::vector<std::string> args;
};

struct Method {
    std::string name;
    std::vector<Command> commands;
};

class ScriptInterpreter {
public:
    ScriptInterpreter();
    bool loadASTFile(const std::string &path);
    void callMethod(const std::string &className, const std::string &methodName);
    void runLoop(const std::string &className, const std::string &updateMethodName = "update", int frames = 200, int ms_per_frame = 16);
private:
    std::map<std::string, Method> methods;
    void executeCommand(const Command &cmd);
};

#endif // SCRIPT_INTERPRETER_H
