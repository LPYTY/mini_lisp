#include "interpreter.h"

void Interpreter::setMode(InterpreterMode m)
{
    mode = m;
}

inline InterpreterMode Interpreter::getMode() const
{
    return mode;
}

void Interpreter::openSource(const string& fileName)
{
    sourceFile.open(fileName);
    if (!sourceFile.is_open())
        throw InterpreterError("Open file \"" + fileName + "\" failed");
    originalCinBuf = cin.rdbuf(sourceFile.rdbuf());
    mode = InterpreterMode::FILEMODE;
}

void Interpreter::closeSource()
{
    if (originalCinBuf)
        cin.rdbuf(originalCinBuf);
    originalCinBuf = nullptr;
    sourceFile.close();
    mode = InterpreterMode::REPLMODE;
}

bool Interpreter::tokenizeAndParseLine()
{
    bool ret = true;
    string line;
    getline(cin, line);
    if (cin.eof())
    {
        ret = false;
        if (line.size() == 0)
        {
            return false;
        }
    }
    auto tokens = Tokenizer::tokenize(line);
    Parser parser(std::move(tokens));
    values.push_back(parser.parse());
    return ret;
}

void Interpreter::cleanUpValueList()
{
    values = {};
}

ValuePtr Interpreter::evalAll()
{
    ValuePtr result;
    while (!values.empty())
    {
        result = globalEvalEnv->eval(values.front());
        values.pop_front();
    }
    return result;
}

void Interpreter::initialize(int argc, const char** argv)
{
    if (argc == 1)
        mode = InterpreterMode::REPLMODE;
    else
    {
        mode = InterpreterMode::FILEMODE;
        openSource(argv[1]);
    }
    exitCode = 0;
    globalEvalEnv = EvalEnv::createGlobal();
}

int Interpreter::run()
{
    while (true)
    {
        try
        {
            if (mode == InterpreterMode::REPLMODE)
                std::cout << ">>> ";
            bool isEOF = tokenizeAndParseLine();
            if (mode == InterpreterMode::REPLMODE)
                if (!values.empty())
                    std::cout << evalAll()->toString() << std::endl;
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
                std::cerr << "SyntaxError: " << e.what() << std::endl;
                cleanUpValueList();
            }
            else
                throw;
        }
        catch (LispError& e)
        {
            if (mode == InterpreterMode::REPLMODE)
            {
                std::cerr << "LispError: " << e.what() << std::endl;
                cleanUpValueList();
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
    closeSource();
}
