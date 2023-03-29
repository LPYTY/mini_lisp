#include "./eval_env.h"

EvalEnv::EvalEnv(EnvPtr parent)
{
    pParent = parent;
    symbolTable.insert(allBuiltins.begin(), allBuiltins.end());
}

EnvPtr EvalEnv::createGlobal()
{
    return EnvPtr(new EvalEnv(nullptr));
}

EnvPtr EvalEnv::createChild(EnvPtr parent, vector<string> names, ValueList values)
{
    auto pEnv = new EvalEnv(parent);
    for (size_t i = 0; i < names.size() && i < values.size(); i++)
    {
        pEnv->symbolTable[names[i]] = values[i];
    }
    return EnvPtr(pEnv);
}

ValuePtr EvalEnv::findValue(const string& name)
{
    try
    {
        return symbolTable.at(name);
    }
    catch (out_of_range& e)
    {
        if (pParent)
            return pParent->findValue(name);
        else
            throw;
    }
}

void EvalEnv::defineVariable(const string& name, ValuePtr value)
{
    symbolTable[name] = value;
}

ValuePtr EvalEnv::eval(ValuePtr expr)
{
    if (expr->isType(ValueType::SelfEvaluatingType))
    {
        return expr;
    }
    else if (expr->isType(ValueType::ListType))
    {
        using namespace std::literals;
        ValueList value = expr->toVector();
        if (value.size() == 0)
            throw LispError("Evaluating nil is prohibited.");
        if (value[0]->asSymbol() == "define"s)
        {
            if (value.size() != 3)
                throw LispError("Malformed define.");
            if (auto name = value[1]->asSymbol())
            {
                defineVariable(*name, eval(value[2]));
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
            return findValue(*name);
        }
        catch (out_of_range& e)
        {
            throw LispError("Variable " + *name + " not defined.");
        }
    }
    throw LispError("Unimplemented");
}

ValueList EvalEnv::evalParams(ValuePtr list)
{
    if (!list->isType(ValueType::ListType))
        throw LispError("Unimplemented");
    ValueList results;
    ValueList v = list->toVector();
    std::transform(
        v.begin() + 1,
        v.end(),
        std::back_inserter(results),
        [this](ValuePtr value) { return this->eval(value); }
    );
    return results;
}

ValuePtr EvalEnv::apply(ValuePtr proc, const ValueList& params)
{
    if (!proc->isType(ValueType::ProcedureType))
        throw LispError("Value isn't callable.");
    return static_pointer_cast<ProcValue>(proc)->call(params, *this);
}

EvalEnv* pCurrentEvalEnv; // Unfinished
