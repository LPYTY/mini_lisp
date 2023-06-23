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
            return "procedure";
        case PromiseType:
            return "promise";
        case CharType:
            return "character";
        case VectorType:
            return "vector";
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

ValuePtr BooleanValue::copy() const
{
    return make_shared<BooleanValue>(bValue);
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

ValuePtr NumericValue::copy() const
{
    return make_shared<NumericValue>(dValue);
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

string StringValue::toDisplayString() const
{
    return value();
}

int StringValue::getTypeID() const
{
    return ValueType::StringType;
}

string& StringValue::value()
{
    return szValue;
}

const string& StringValue::value() const
{
    return szValue;
}

char& StringValue::at(long long index)
{
    if (index < 0 || index >= szValue.size())
        throw LispError("Index " + to_string(index) + " out of range");
    return szValue[index];
}

ValuePtr StringValue::copy() const
{
    return make_shared<StringValue>(szValue);
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

ValuePtr NilValue::copy() const
{
    return make_shared<NilValue>();
}

string NilValue::extractString(bool isOnRight) const
{
    return isOnRight ? "" : "()";
}

string NilValue::extractDisplayString(bool isOnRight) const
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

ValuePtr SymbolValue::copy() const
{
    return make_shared<SymbolValue>(szSymbolName);
}

string PairValue::toString() const
{
    return '(' + extractString(false) + ')';
}

string PairValue::toDisplayString() const
{
    return '(' + extractDisplayString(false) + ')';
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

ValuePtr PairValue::copy() const
{
    return make_shared<PairValue>(pLeftValue->copy(), pRightValue->copy());
}

string PairValue::extractString(bool isOnRight) const
{
    return (isOnRight ? " " : "") + pLeftValue->toString() + pRightValue->extractString(true);
}

string PairValue::extractDisplayString(bool isOnRight) const
{
    return (isOnRight ? " " : "") + pLeftValue->toDisplayString() + pRightValue->extractDisplayString(true);
}

string Value::toDisplayString() const
{
    return toString();
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

string Value::extractDisplayString(bool isOnRight) const
{
    return (isOnRight ? " . " : "") + toDisplayString();
}

const vector<int> CallableValue::UnlimitedType{ ValueType::AllType, CallableValue::SameToRest };

ValuePtr CallableValue::call(const ValueList& args, EvalEnv& env)
{
    checkValidParamCnt(args);
    checkValidParamType(args);
    return proc(args, env);
}

void CallableValue::assertParamCnt(const ValueList& params, int minArgs, int maxArgs)
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

void CallableValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt, maxParamCnt);
}

void CallableValue::checkValidParamType(const ValueList& params)
{
    if (paramType.size() == 0)
        return;
    bool hasRestType = false;
    int restType = 0;
    size_t checkSize = paramType.size();
    for (size_t i = 0; i < paramType.size(); i++)
    {
        if (paramType[i] == SameToRest)
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
        CallableValue::assertParamCnt(params, minArgs, maxArgs);
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

ValuePtr BuiltinProcValue::copy() const
{
    return make_shared<BuiltinProcValue>(proc, minParamCnt, maxParamCnt, paramType);
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

string SpecialFormValue::toString() const
{
    throw LispError("Cannot convert a special form to string.");
}

void SpecialFormValue::assertParamCnt(const ValueList& params, int minArgs, int maxArgs)
{
    try
    {
        CallableValue::assertParamCnt(params, minArgs, maxArgs);
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

ValuePtr SpecialFormValue::copy() const
{
    return make_shared<SpecialFormValue>(proc, minParamCnt, maxParamCnt, paramType);
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
    :ProcValue (standardLambdaProc, paramsDefinition.size(), paramsDefinition.size()), paramNames(paramsDefinition), body(bodyDefinition), parentEnv(parentEvalEnv)
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

ValuePtr LambdaValue::copy() const
{
    return make_shared<LambdaValue>(paramNames, body, parentEnv);
}

void LambdaValue::checkValidParamCnt(const ValueList& params)
{
    assertParamCnt(params, minParamCnt);
}

EnvPtr LambdaValue::prepareEvalEnv(const ValueList& params)
{
    return EvalEnv::createChild(parentEnv, paramNames, params);
}

string ProcValue::toString() const
{
    return "#procedure";
}

int PromiseValue::getTypeID() const
{
    return ValueType::PromiseType;
}

string PromiseValue::toString() const
{
    return "#promise";
}

ValuePtr PromiseValue::force(EvalEnv& env)
{
    if (isEvaluated)
        return value;
    else
    {
        value = env.eval(value);
        isEvaluated = true;
    }
    return value;
}

ValuePtr PromiseValue::copy() const
{
    auto result = make_shared<PromiseValue>(value->copy());
    result->isEvaluated = isEvaluated;
    return result;
}

/*
const vector<int> ParamChecker::UnlimitedType = {};

bool ParamChecker::isValid(const ValueList& params)
{
    auto size = params.size();

    return false;
}
*/

string CharValue::toString() const
{
    using namespace std::literals;
    if (cValue == ' ')
        return "#\\space"s;
    else if (cValue == '\n')
        return "#\\newline"s;
    else
        return "#\\"s + cValue;
}

string CharValue::toDisplayString() const
{
    return string(1, value());
}

int CharValue::getTypeID() const
{
    return ValueType::CharType;
}

char CharValue::value() const
{
    return cValue;
}

ValuePtr CharValue::copy() const
{
    return make_shared<CharValue>(cValue);
}

string VectorValue::toString() const
{
    string result = "#(";
    for (size_t i = 0; i < vecValue.size(); i++)
    {
        result += vecValue[i]->toString();
        if (i != vecValue.size() - 1)
        {
            result += ' ';
        }
    }
    result += ')';
    return result;
}

string VectorValue::toDisplayString() const
{
    string result = "#(";
    for (size_t i = 0; i < vecValue.size(); i++)
    {
        result += vecValue[i]->toDisplayString();
        if (i != vecValue.size() - 1)
        {
            result += ' ';
        }
    }
    result += ')';
    return result;
}

int VectorValue::getTypeID() const
{
    return ValueType::VectorType;
}

vector<ValuePtr>& VectorValue::value()
{
    return vecValue;
}

ValuePtr& VectorValue::at(long long index)
{
    if (index < 0 || index >= vecValue.size())
        throw LispError("Index " + to_string(index) + " out of range");
    return vecValue[index];
}

ValuePtr VectorValue::copy() const
{
    ValueList result;
    std::ranges::transform(vecValue, std::back_inserter(result), [](auto& a) {return a->copy(); });
    return make_shared<VectorValue>(result);
}
