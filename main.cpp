#include <iostream>
#include <string>

#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"
#include "./interpreter.h"

#if defined(DEBUG) || defined(_DEBUG)
#define __ENABLE_TEST
#endif // defined(DEBUG) || defined(_DEBUG)

#define __DO_RJSJ_TEST

#ifdef __ENABLE_TEST
#include "./rjsj_test.hpp"

struct TestCtx 
{
    EnvPtr env = EvalEnv::createGlobal();
    std::string eval(std::string input) 
    {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        auto value = parser.parse();
        auto result = env->eval(std::move(value));
        return result->toString();
    }
};
#endif //__ENABLE_TEST

int main(int argc, const char ** argv) 
{
#ifdef __DO_RJSJ_TEST
    RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6, Lv7, Lv7Lib);
#endif //__DO_RJSJ_TEST

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

