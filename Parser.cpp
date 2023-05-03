#include "./parser.h"

Parser::Parser(TokenList&& tokenList)
{
    tokens = std::move(tokenList);
}

ValuePtr Parser::parse()
{
    auto&& token = popNextToken();
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
    case TokenType::QUOTE:
    case TokenType::QUASIQUOTE:
    case TokenType::UNQUOTE:
    case TokenType::UNQUOTE_SPLICING:
    {
        return make_shared<PairValue>(substituteSymbol(token),make_shared<PairValue>(parse(),make_shared<NilValue>()));
    }
    default:
        throw SyntaxError("Unimplemented");
        break;
    }
    return ValuePtr();
}

TokenPtr Parser::popNextToken()
{
    if (tokens.empty())
        throw SyntaxError("More token(s) expected");
    auto nextToken = std::move(tokens.front());
    tokens.pop_front();
    return nextToken;
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
        tokens.pop_front();
    }
    else
    {
        cdr = parseTails();
    }
    return make_shared<PairValue>(car, cdr);
}

ValuePtr Parser::substituteSymbol(TokenPtr& token) const
{
    string name;
    switch (token->getType())
    {
    case TokenType::QUOTE:
    {
        name = "quote";
        break;
    }
    case TokenType::QUASIQUOTE:
    {
        name = "quasiquote";
        break;
    }
    case TokenType::UNQUOTE:
    {
        name = "unquote";
        break;
    }
    case TokenType::UNQUOTE_SPLICING:
    {
        name = "unquote-splicing";
        break;
    }
    default:
        name = "";
        break;
    }
    return make_shared<SymbolValue>(name);
}
