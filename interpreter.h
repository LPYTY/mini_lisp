#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <fstream>
#include <memory>
#include <deque>

#include "./error.h"
#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"
#include "./reader.h"

using std::istream, std::cin, std::cout, std::cerr, std::endl, std::ifstream, std::string, std::streambuf, std::shared_ptr, std::make_shared, std::deque;

enum InterpreterMode
{
    FILEMODE,
    REPLMODE
};

class Interpreter
{
    InterpreterMode mode;
    EnvPtr globalEvalEnv;
    int exitCode;
    shared_ptr<Reader> codeReader;
private:
    InterpreterMode getMode() const;
    ValueList evalAll();
    Interpreter();
    Interpreter(const string& fileName);
public:
    static shared_ptr<Interpreter> createInterpreter(int argc, const char** argv);
    int run();
    ~Interpreter();
};

#endif // !INTERPRETER_H



