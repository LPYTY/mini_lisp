#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <optional>
#include <deque>
#include <memory>
#include <string>
#include <string_view>

#include "./token.h"
#include "./tokenizer.h"
#include "./parser.h"
#include "./error.h"

class Reader
{
    std::istream* pSource;
    std::ifstream sourceFile;
    std::string currentLine;
    std::deque<ValuePtr> values;
public:
    Reader()
        :pSource{ &std::cin } {}
    Reader(const std::string& fileName);
    ValuePtr read();
    std::deque<ValuePtr>& getAllValues();
    bool isEmpty();
    bool tokenizeAndParseLine();
    void cleanUpValueList();

private:
    std::istream& inputStream();

};

extern shared_ptr<Reader> stdinReader;

#endif // !READER_H


