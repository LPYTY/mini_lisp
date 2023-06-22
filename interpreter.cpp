#include "interpreter.h"

inline InterpreterMode Interpreter::getMode() const
{
    return mode;
}

ValueList Interpreter::evalAll()
{
    ValueList result;
    auto& values = codeReader->getAllValues();
    while (!values.empty())
    {
        ValuePtr value = values.front();
        values.pop_front();
        result.push_back(globalEvalEnv->eval(value));
    }
    return result;
}

Interpreter::Interpreter()
    :mode{ InterpreterMode::REPLMODE }, exitCode{ 0 }, codeReader{stdinReader}
{
    globalEvalEnv = EvalEnv::createGlobal();
}

Interpreter::Interpreter(const string& fileName)
    :mode{ InterpreterMode::FILEMODE }, exitCode{ 0 }
{
    codeReader = make_shared<Reader>(fileName);
    globalEvalEnv = EvalEnv::createGlobal();
}

shared_ptr<Interpreter> Interpreter::createInterpreter(int argc, const char** argv)
{
    if (argc == 1)
        return shared_ptr<Interpreter>(new Interpreter);
    else
        return shared_ptr<Interpreter>(new Interpreter(argv[1]));
}

int Interpreter::run()
{
    while (true)
    {
        try
        {
            if (mode == InterpreterMode::REPLMODE)
                cout << ">>> ";
            bool isEOF = codeReader->tokenizeAndParseLine();
            if (mode == InterpreterMode::REPLMODE)
            {
                auto values = evalAll();
                for (auto value : values)
                {
                    cout << value->toString() << endl;
                }
            }
            if(!isEOF)
                break;
        }
        catch (ExitEvent& e)
        {
            exitCode = e.exitCode();
            break;
        }
        catch (SyntaxError& e)
        {
            if (mode == InterpreterMode::REPLMODE)
            {
                cerr << "SyntaxError: " << e.what() << endl;
                codeReader->cleanUpValueList();
            }
            else
                throw;
        }
        catch (LispError& e)
        {
            if (mode == InterpreterMode::REPLMODE)
            {
                std::cerr << "LispError: " << e.what() << std::endl;
                codeReader->cleanUpValueList();
            }
            else
                throw;
        }
    }
    if (mode == InterpreterMode::FILEMODE)
    {
        try
        {
            evalAll();
        }
        catch (ExitEvent& e)
        {
            exitCode = e.exitCode();
        }
    }
    return exitCode;
}

Interpreter::~Interpreter()
{
}
