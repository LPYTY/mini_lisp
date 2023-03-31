#include "./eval_env.h"

const unordered_map<string, ProcPtr>& EvalEnv::specialFormTable = allSpecialForms;

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

ProcPtr EvalEnv::findForm(const string& name)
{
    try
    {
        return specialFormTable.at(name);
    }
    catch (out_of_range& e)
    {
            throw LispError("");
    }
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
            throw LispError("");
    }
}

void EvalEnv::defineVariable(const string& name, ValuePtr value)
{
    symbolTable[name] = value;
}

void EvalEnv::undefVariable(const string& name)
{
    symbolTable.erase(name);
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
        ValuePtr proc = eval(value[0]);
        if (!proc->isType(ValueType::ProcedureType))
            throw LispError("Not a procedure " + proc->toString());
        return apply(proc, static_pointer_cast<PairValue>(expr)->right());
    }
    else if (auto name = expr->asSymbol())
    {
        try
        {
            return findForm(*name);
        }
        catch(LispError& e)
        {
            try
            {
                return findValue(*name);
            }
            catch (LispError& e)
            {
                throw LispError("Variable " + *name + " not defined.");
            }
        }
    }
    throw LispError("Unimplemented");
}

ValueList EvalEnv::evalParams(const ValueList& list)
{
    ValueList results;
    std::ranges::transform(
        list,
        std::back_inserter(results),
        [this](ValuePtr value) { return this->eval(value); }
    );
    return results;
}

ValueList EvalEnv::evalParams(ValuePtr list)
{
    ValueList v = list->toVector();
    return evalParams(v);
}

ValuePtr EvalEnv::apply(ValuePtr proc, ValuePtr params)
{
    ValueList args;
    if (!proc->isType(ValueType::SpecialFormType))
        args = evalParams(params);
    else
    {
        auto paramList = params->toVector();
        args.insert(args.end(), paramList.begin(), paramList.end());
    }
    return static_pointer_cast<ProcValue>(proc)->call(args, *this);
}

ValuePtr EvalEnv::applyProc(ValuePtr proc, const ValueList& params)
{
    return static_pointer_cast<ProcValue>(proc)->call(evalParams(params), *this);
}
