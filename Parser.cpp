#include "Parser.h"

Parser::Parser(TokenList&& tokenList)
{
    tokens = std::move(tokenList);
}

ValuePtr Parser::parse()
{
    auto& token = tokens.front();
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
    default:
        throw SyntaxError("Unimplemented");
        break;
    }
    return ValuePtr();
}
