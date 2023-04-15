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
    pSource = &sourceFile;
}

istream& Interpreter::inputStream()
{
    return *pSource;
}

bool Interpreter::tokenizeAndParseLine()
{
    bool ret = true;
    string line;
    getline(inputStream(), line);
    if (inputStream().eof())
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

Interpreter::Interpreter()
    :mode{ InterpreterMode::REPLMODE }, pSource{ &cin }, sourceFile{}, exitCode{ 0 }
{
    globalEvalEnv = EvalEnv::createGlobal();
}

Interpreter::Interpreter(const string& fileName)
    :mode{ InterpreterMode::FILEMODE }, pSource{ nullptr }, sourceFile{}, exitCode{ 0 }
{
    openSource(fileName);
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
            bool isEOF = tokenizeAndParseLine();
            if (mode == InterpreterMode::REPLMODE)
                if (!values.empty())
                    cout << evalAll()->toString() << endl;
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
}
