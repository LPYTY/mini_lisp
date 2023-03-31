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

using std::istream, std::cin, std::cout, std::endl, std::ifstream, std::string, std::streambuf, std::shared_ptr, std::make_shared, std::deque;

enum InterpreterMode
{
    FILEMODE,
    REPLMODE
};

class Interpreter
{
    InterpreterMode mode;
    ifstream sourceFile;
    streambuf* originalCinBuf;
    EnvPtr globalEvalEnv;
    int exitCode;
    deque<ValuePtr> values;
private:
    void setMode(InterpreterMode m);
    InterpreterMode getMode() const;
    void openSource(const string& fileName);
    void closeSource();
    bool tokenizeAndParseLine();
    void cleanUpValueList();
    ValuePtr evalAll();
public:
    Interpreter()
        :mode(InterpreterMode::REPLMODE), originalCinBuf(nullptr), exitCode(0) {}
    void initialize(int argc, const char ** argv);
    int run();
    ~Interpreter();
};

#endif // !INTERPRETER_H



