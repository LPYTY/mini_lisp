#include "./eval_env.h"

EvalEnv::EvalEnv()
{
    symbolTable.insert(allBuiltins.begin(), allBuiltins.end());
}

ValuePtr EvalEnv::eval(ValuePtr expr)
{
    if (expr->isSelfEvaluating())
    {
        return expr;
    }
    else if (expr->isList())
    {
        using namespace std::literals;
        vector<ValuePtr> value = expr->toVector();
        if (value.size() == 0)
            throw LispError("Evaluating nil is prohibited.");
        if (value[0]->asSymbol() == "define"s)
        {
            if (value.size() != 3)
                throw LispError("Malformed define.");
            if (auto name = value[1]->asSymbol())
            {
                symbolTable[*name] = eval(value[2]);
                return make_shared<NilValue>();
            }
            else
            {
                throw LispError("Malformed define.");
            }
        }
        ValuePtr proc = eval(value[0]);
        auto args = evalParams(expr);
        return apply(proc, args);
    }
    else if (auto name = expr->asSymbol())
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
    throw LispError("Unimplemented");
}

vector<ValuePtr> EvalEnv::evalParams(ValuePtr list)
{
    if (!list->isList())
        throw LispError("Unimplemented");
    vector<ValuePtr> results;
    vector<ValuePtr> v = list->toVector();
    std::transform(
        v.begin() + 1,
        v.end(),
        std::back_inserter(results),
        [this](ValuePtr value) { return this->eval(value); }
    );
    return results;
}

ValuePtr EvalEnv::apply(ValuePtr proc, const vector<ValuePtr>& params)
{
    if (!proc->isCallable())
        throw LispError("Value isn't callable.");
    return static_pointer_cast<ProcValue>(proc)->call(params);
}
