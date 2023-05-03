#include "./eval_env.h"

EvalEnv::EvalEnv(EnvPtr parent)
    :pParent{parent}
{
    symbolTable.insert(allBuiltins.begin(), allBuiltins.end());
    specialFormTable.insert(allSpecialForms.begin(), allSpecialForms.end());
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

pair<EnvPtr, FormPtr> EvalEnv::findForm(const string& name)
{
    EnvPtr currentEnv = shared_from_this();
    pair<EnvPtr, FormPtr> result = { nullptr, nullptr };
    while (currentEnv)
    {
        auto iter = currentEnv->specialFormTable.find(name);
        if (iter != currentEnv->specialFormTable.end())
        {
            result.first = currentEnv;
            result.second = iter->second;
            break;
        }
        currentEnv = currentEnv->pParent;
    }
    return result;
}

FormPtr EvalEnv::getForm(const string& name)
{
    auto result = findForm(name);
    if (result.second)
        return result.second;
    else
        throw LispError("Variable " + name + " not defined.");
    
}

pair<EnvPtr, ValuePtr> EvalEnv::findVariable(const string& name)
{
    EnvPtr currentEnv = shared_from_this();
    pair<EnvPtr, ValuePtr> result = { nullptr, nullptr };
    while (currentEnv)
    {
        auto iter = currentEnv->symbolTable.find(name);
        if (iter != currentEnv->symbolTable.end())
        {
            result.first = currentEnv;
            result.second = iter->second;
            break;
        }
        currentEnv = currentEnv->pParent;
    }
    return result;
}

ValuePtr EvalEnv::getVariableValue(const string& name)
{
    auto result = findVariable(name);
    if (result.second)
        return result.second;
    else
        throw LispError("Variable " + name + " not defined.");
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
        if (proc->isType(ValueType::ProcedureType))
            return apply(proc, static_pointer_cast<PairValue>(expr)->right());
        else if (proc->isType(ValueType::SpecialFormType))
            return callForm(proc, static_pointer_cast<PairValue>(expr)->right());
        else
            throw LispError("Not a procedure " + proc->toString());
    }
    else if (auto name = expr->asSymbol())
    {
        try
        {
            return getForm(*name);
        }
        catch (LispError& e)
        {
            return getVariableValue(*name);
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
    return apply(proc, params->toVector());
}

ValuePtr EvalEnv::callForm(ValuePtr form, ValuePtr params)
{
    return static_pointer_cast<SpecialFormValue>(form)->call(params->toVector(), *this);
}

ValuePtr EvalEnv::apply(ValuePtr proc, const ValueList& params)
{
    return static_pointer_cast<ProcValue>(proc)->call(evalParams(params), *this);
}
