#include <iostream>
#include <string>

#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"
#include "./interpreter.h"

int main(int argc, const char ** argv) 
{
    std::shared_ptr<Interpreter> interpreter = Interpreter::createInterpreter(argc, argv);
    int exitCode = 0;
    try
    {
        exitCode = interpreter->run();
    }
    catch (SyntaxError& e)
    {
        std::cerr << "SyntaxError: " << e.what() << std::endl;
        exitCode = -1;
    }
    catch (LispError& e)
    {
        std::cerr << "LispError: " << e.what() << std::endl;
        exitCode = -2;
    }
    catch (InterpreterError& e)
    {
        std::cerr << "InterpreterError: " << e.what() << std::endl;
        exitCode = -3;
    }
    return exitCode;
}

