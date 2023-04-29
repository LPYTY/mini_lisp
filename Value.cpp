#include "./value.h"
#include "./eval_env.h"

namespace ValueType
{
    string typeName(int typeID)
    {
        switch (typeID)
        {
        case BooleanType:
            return "boolean";
        case NumericType:
            return "number";
        case StringType:
            return "string";
        case NilType:
            return "nil";
        case SymbolType:
            return "symbol";
        case PairType:
            return "pair";
        case BuiltinProcType:
            return "builtin procedure";
        case SpecialFormType:
            return "special form";
        case ListType:
            return "list";
        case ProcedureType:
        case FunctionType:
            return "procedure";
        default:
            break;
        }
        return "";
    }
}

ostream& operator<<(ostream& os, const Value& thisValue)
{
    os << thisValue.toString() << endl;
    return os;
}

string BooleanValue::toString() const
{
    return bValue ? "#t" : "#f";
}

int BooleanValue::getTypeID() const
{
    return ValueType::BooleanType;
}

BooleanValue::operator bool()
{
    return bValue;
}

bool NumericValue::isInteger() const
{
    return static_cast<long long>(dValue) == dValue;
}

string NumericValue::toString() const
{
    return isInteger() ? to_string(static_cast<int>(dValue)) : to_string(dValue);
}

int NumericValue::getTypeID() const
{
    return ValueType::NumericType;
}

optional<double> NumericValue::asNumber() const
{
    return dValue;
}

string StringValue::escChars = { '\"', '\\' };

string StringValue::toString() const
{
    vector<size_t> escPos;
    string result = "\"";
    for (size_t lastPos = 0; (lastPos = szValue.find_first_of(escChars, lastPos)) != string::npos; lastPos++)
    {
        escPos.push_back(lastPos);
    }
    for (size_t startPos = 0, i = 0, endPos = 0; i <= escPos.size(); i++)
    {
        if (i < escPos.size())
        {
            endPos = escPos[i];
        }
        else
        {
            endPos = szValue.size();
        }
        result += std::string_view(szValue.c_str() + startPos, endPos - startPos);
        if (i < escPos.size())
        {
            result += "\\";
            startPos = endPos;
        }
    }
    result += "\"";
    return result;
}

int StringValue::getTypeID() const
{
    return ValueType::StringType;
}

const string& StringValue::value() const
{
    return szValue;
}

string NilValue::toString() const
{
    return "()";
}

int NilValue::getTypeID() const
{
    return ValueType::NilType;
}

ValueList NilValue::toVector()
{
    return ValueList();
}

bool NilValue::isList()
{
    return true;
}

string NilValue::extractString(bool isOnRight) const
{
    return isOnRight ? "" : "()";
}

string SymbolValue::toString() const
{
    return szSymbolName;
}

int SymbolValue::getTypeID() const
{
    return ValueType::SymbolType;
}

optional<string> SymbolValue::asSymbol() const
{
    return szSymbolName;
}

string PairValue::toString() const
{
    return '(' + extractString(false) + ')';
}

int PairValue::getTypeID() const
{
    return ValueType::PairType;
}

shared_ptr<ListValue> ListValue::fromVector(const ValueList& v)
{
    return createListFromIter(v.begin(), v.end());
}

shared_ptr<ListValue> ListValue::fromDeque(deque<ValuePtr>& q)
{
    return createListFromIter(q.begin(), q.end());
}

ValueList PairValue::toVector()
{
    if (!pRightValue->isType(ValueType::ListType))
        throw LispError("Malformed list: expected pair or nil, got " + pRightValue->toString());
    ValueList leftVector, rightVector = pRightValue->toVector();
    leftVector.push_back(pLeftValue);
    leftVector.insert(leftVector.end(), rightVector.begin(), rightVector.end());
    return leftVector;
}

ValuePtr PairValue::left()
{
    return pLeftValue;
}

ValuePtr PairValue::right()
{
    return pRightValue;
}

bool PairValue::isList()
{
    return pRightValue->isType(ValueType::NilType) || pRightValue->isType(ValueType::PairType) && static_pointer_cast<PairValue>(pRightValue)->isList();
}

string PairValue::extractString(bool isOnRight) const
{
    return (isOnRight ? " " : "") + pLeftValue->toString() + pRightValue->extractString(true);
}

bool Value::isType(int typeID) const
{
    return getTypeID() & typeID;
}

ValueList Value::toVector()
{
    throw LispError("Malformed list: expected pair or nil, got " + toString() + ".");
}

optional<string> Value::asSymbol() const
{
    return nullopt;
}

optional<double> Value::asNumber() const
{
    return nullopt;
}

Value::operator bool()
{
    return true;
}

string Value::extractString(bool isOnRight) const
{
    return (isOnRight ? " . " : "") + toString();
}

const vector<int> ProcValue::UnlimitedType{ ValueType::AllType, ProcValue::SameToRest };

ValuePtr ProcValue::call(const ValueList& args, EvalEnv& env)
{
    checkValidParamCnt(args);
    checkValidParamType(args);
    return proc(args, env);
}

string ProcValue::toString() const
{
    return "#procedure";
}

void ProcValue::assertParamCnt(const ValueList& params, int minArgs, int maxArgs)
{
    if (minArgs != UnlimitedCnt && params.size() < minArgs)
    {
        throw TooFewArgumentsError("");
    }
    if (maxArgs != UnlimitedCnt && params.size() > maxArgs)
    {
        throw TooManyArgumentsError("");
    }
}

void ProcValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt, maxParamCnt);
}

void ProcValue::checkValidParamType(const ValueList& params)
{
    if (paramType.size() == 0)
        return;
    bool hasRestType = false;
    int restType = 0;
    size_t checkSize = paramType.size();
    for (size_t i = 0; i < paramType.size(); i++)
    {
        if (paramType[i] = SameToRest)
        {
            if (i == 0)
                return;
            checkSize = i;
            hasRestType = true;
            restType = paramType[i - 1];
            break;
        }
    }
    for (size_t i = 0; i < params.size(); i++)
    {
        if (i < checkSize)
        {
            if (!params[i]->isType(paramType[i]))
                throw LispError(params[i]->toString() + " is not " + ValueType::typeName(paramType[i]));
        }
        else
        {
            if (!hasRestType)
                return;
            if (!params[i]->isType(restType))
                throw LispError(params[i]->toString() + " is not " + ValueType::typeName(restType));
        }
    }
}

int BuiltinProcValue::getTypeID() const
{
    return ValueType::BuiltinProcType;
}

void BuiltinProcValue::assertParamCnt(const ValueList& params, int minArgs, int maxArgs)
{
    try
    {
        ProcValue::assertParamCnt(params, minArgs, maxArgs);
    }
    catch (TooFewArgumentsError& e)
    {
        throw LispError("Too few arguments: " + to_string(params.size()) + " < " + to_string(minArgs));
    }
    catch (TooManyArgumentsError& e)
    {
        throw LispError("Too many arguments: " + to_string(params.size()) + " > " + to_string(maxArgs));
    }
}

void BuiltinProcValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt, maxParamCnt);
}

bool ListValue::isEmpty()
{
    return toVector().size() == 0;
}

int SpecialFormValue::getTypeID() const
{
    return ValueType::SpecialFormType;
}

void SpecialFormValue::assertParamCnt(const ValueList& params, int minArgs, int maxArgs)
{
    try
    {
        ProcValue::assertParamCnt(params, minArgs, maxArgs);
    }
    catch (TooFewArgumentsError& e)
    {
        throw LispError("Too few operands: " + to_string(params.size()) + " < " + to_string(minArgs));
    }
    catch (TooManyArgumentsError& e)
    {
        throw LispError("Too many operands: " + to_string(params.size()) + " > " + to_string(maxArgs));
    }
}

void SpecialFormValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt, maxParamCnt);
}

ValuePtr LambdaValue::standardLambdaProc(const ValueList& values, EvalEnv& env)
{
    ValuePtr result;
    for (auto& value : values)
        result = env.eval(value);
    return result;
}

LambdaValue::LambdaValue(const vector<string>& paramsDefinition, const ValueList& bodyDefinition, EnvPtr parentEvalEnv)
    :ProcValue(standardLambdaProc, paramsDefinition.size(), paramsDefinition.size()), paramNames(paramsDefinition), body(bodyDefinition), parentEnv(parentEvalEnv)
{
}

int LambdaValue::getTypeID() const
{
    return ValueType::LambdaType;
}

ValuePtr LambdaValue::call(const ValueList& params, EvalEnv& env)
{
    checkValidParamCnt(params);
    auto lambdaEnv = prepareEvalEnv(params);
    return proc(body, *lambdaEnv);
}

void LambdaValue::assertParamCnt(const ValueList& params, int argCnt)
{
    if (params.size() != argCnt)
        throw LispError("Procedure expected " + to_string(argCnt) + " parameters, got " + to_string(params.size()));
}

void LambdaValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt);
}

EnvPtr LambdaValue::prepareEvalEnv(const ValueList& params)
{
    return EvalEnv::createChild(parentEnv, paramNames, params);
}
