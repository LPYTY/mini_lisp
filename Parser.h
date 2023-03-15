#ifndef PARSER_H
#define PARSER_H

#include <list>
#include <deque>
#include <memory>
#include <type_traits>

#include ".\token.h"
#include ".\Value.h"
#include ".\error.h"

using std::list, std::deque, std::unique_ptr, std::derived_from;

using TokenList = deque<TokenPtr>;

class Parser
{
    TokenList tokens;
public:
    Parser(TokenList&& tokenList);
    ValuePtr parse();
};

#endif

