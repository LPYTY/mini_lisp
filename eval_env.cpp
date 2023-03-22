#include "./eval_env.h"

ValuePtr EvalEnv::eval(ValuePtr expr)
{
    if (expr->isSelfEvaluating())
    {
        return expr;
    }
    if (expr->isNil())
    {
        throw LispError("Evaluating nil is prohibited.");
    }
    if (expr->isList())
    {
        using namespace std::literals;
        vector<ValuePtr> value = expr->toVector();
        if (value[0]->asSymbol() == "define"s)
        {
            if (value.size() != 4)
                throw LispError("Malformed define.");
            if (auto name = value[1]->asSymbol())
            {
                symbolTable[*name] = value[2];
                return make_shared<NilValue>();
            }
            else
            {
                throw LispError("Malformed define.");
            }
        }
    }
    if (auto name = expr->asSymbol())
    {
        try
        {
            return symbolTable.at(*name);
        }
        catch (out_of_range& e)
        {
            throw LispError("Variable " + *name + " not defined.");
        }
    }
    throw LispError("Unimplemented.");
}
