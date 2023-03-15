#include "Parser.h"

Parser::Parser(TokenList&& tokenList)
{
    tokens = std::move(tokenList);
}

ValuePtr Parser::parse()
{
    auto token = std::move(tokens.front());
    tokens.pop_front();
    switch (token->getType())
    {
    case TokenType::NUMERIC_LITERAL:
    {
        auto value = static_cast<NumericLiteralToken&>(*token).getValue();
        return std::make_shared<NumericValue>(value);
        break;
    }
    case TokenType::BOOLEAN_LITERAL:
    {
        auto value = static_cast<BooleanLiteralToken&>(*token).getValue();
        return std::make_shared<BooleanValue>(value);
        break;
    }
    case TokenType::STRING_LITERAL:
    {
        auto& value = static_cast<StringLiteralToken&>(*token).getValue();
        return std::make_shared<StringValue>(value);
        break;
    }
    case TokenType::IDENTIFIER:
    {
        auto& value = static_cast<IdentifierToken&>(*token).getName();
        return std::make_shared<SymbolValue>(value);
        break;
    }
    case TokenType::LEFT_PAREN:
    {
        return parseTails();
        break;
    }
    default:
        throw SyntaxError("Unimplemented");
        break;
    }
    return ValuePtr();
}

TokenPtr& Parser::getNextToken()
{
    if (tokens.empty())
        throw SyntaxError("More token(s) expected");
    return tokens.front();
}

ValuePtr Parser::parseTails()
{
    if (getNextToken()->getType() == TokenType::RIGHT_PAREN)
    {
        tokens.pop_front();
        return make_shared<NilValue>();
    }
    auto car = parse();
    ValuePtr cdr;
    if (getNextToken()->getType() == TokenType::DOT)
    {
        tokens.pop_front();
        cdr = parse();
        if (getNextToken()->getType() != TokenType::RIGHT_PAREN)
        {
            throw SyntaxError("Right paren expected");
        }
    }
    else
    {
        cdr = parseTails();
    }
    return make_shared<PairValue>(car, cdr);
}
