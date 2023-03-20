#ifndef PARSER_H
#define PARSER_H

#include <deque>
#include <memory>

#include "./token.h"
#include "./value.h"
#include "./error.h"

using std::deque, std::make_shared;

using TokenList = deque<TokenPtr>;

class Parser
{
    TokenList tokens;
public:
    Parser(TokenList&& tokenList);
    ValuePtr parse();
private:
    TokenPtr popNextToken();
    TokenPtr& getNextToken();
    ValuePtr parseTails();
    ValuePtr substituteSymbol(TokenPtr& token) const;
};

#endif

