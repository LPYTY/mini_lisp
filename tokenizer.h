#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <deque>
#include <string>

#include "./token.h"

class Tokenizer 
{
private:
    TokenPtr nextToken(int& pos);
    std::deque<TokenPtr> tokenize();

    //std::string input;
    std::shared_ptr<std::string> pInput;
    Tokenizer(const std::string& input) : pInput{new std::string(input)} {}

public:
    static std::deque<TokenPtr> tokenize(const std::string& input);
};

#endif
