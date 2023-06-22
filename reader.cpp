#include "reader.h"

Reader::Reader(const std::string& fileName)
{
    sourceFile.open(fileName);
    if (!sourceFile.is_open())
        throw InterpreterError("Open file \"" + fileName + "\" failed");
    pSource = &sourceFile;
}

ValuePtr Reader::read()
{
    while (isEmpty())
        tokenizeAndParseLine();
    auto result = values[0];
    values.pop_front();
    return result;
}

std::deque<ValuePtr>& Reader::getAllValues()
{
    return values;
}

bool Reader::isEmpty()
{
    return values.empty();
}

bool Reader::tokenizeAndParseLine()
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
    while (!parser.isEmpty())
        values.push_back(parser.parse());
    return ret;
}

void Reader::cleanUpValueList()
{
    values = {};
}

std::istream& Reader::inputStream()
{
    return *pSource;
}

std::shared_ptr<Reader> stdinReader = std::make_shared<Reader>();
