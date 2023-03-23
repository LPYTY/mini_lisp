#include "./value.h"

ostream& operator<<(ostream& os, const Value& thisValue)
{
    os << thisValue.toString() << endl;
    return os;
}

string BooleanValue::toString() const
{
    return bValue ? "#t" : "#f";
}

string BooleanValue::getTypeName() const
{
    return "boolean value";
}

bool NumericValue::isInteger() const
{
    return static_cast<int>(dValue) == dValue;
}

string NumericValue::toString() const
{
    return isInteger() ? to_string(static_cast<int>(dValue)) : to_string(dValue);
}

string NumericValue::getTypeName() const
{
    return "numeric value";
}

optional<double> NumericValue::asNumber() const
{
    return dValue;
}

string StringValue::toString() const
{
    return '"' + szValue + '"';
}

string StringValue::getTypeName() const
{
    return "string value";
}

string NilValue::toString() const
{
    return "()";
}

string NilValue::getTypeName() const
{
    return "nil value";
}

vector<ValuePtr> NilValue::toVector()
{
    return vector<ValuePtr>();
}

string NilValue::extractString(bool isOnRight) const
{
    return "";
}

string SymbolValue::toString() const
{
    return szSymbolName;
}

string SymbolValue::getTypeName() const
{
    return "symbol value";
}

optional<string> SymbolValue::asSymbol() const
{
    return szSymbolName;
}

string PairValue::toString() const
{
    return '(' + extractString(false) + ')';
}

string PairValue::getTypeName() const
{
    return "pair value";
}

shared_ptr<PairValue> PairValue::fromVector(vector<ValuePtr>& v)
{
    return fromIter(v.begin(), v.end());
}

shared_ptr<PairValue> PairValue::fromDeque(deque<ValuePtr>& q)
{
    return fromIter(q.begin(), q.end());
}

vector<ValuePtr> PairValue::toVector()
{
    vector<ValuePtr> leftVector, rightVector = pRightValue->toVector();
    leftVector.push_back(pLeftValue);
    leftVector.insert(leftVector.end(), rightVector.begin(), rightVector.end());
    return leftVector;
}

string PairValue::extractString(bool isOnRight) const
{
    return (isOnRight ? " " : "") + pLeftValue->extractString(false) + pRightValue->extractString(true);
}

bool Value::isSelfEvaluating() const
{
    return false;
}

bool Value::isNil()
{
    return toVector().size() == 0;
}

bool Value::isList() const
{
    return false;
}

bool Value::isCallable() const
{
    return false;
}

vector<ValuePtr> Value::toVector()
{
    return vector<ValuePtr>{shared_from_this()};
}

optional<string> Value::asSymbol() const
{
    return nullopt;
}

optional<double> Value::asNumber() const
{
    return nullopt;
}

string Value::extractString(bool isOnRight) const
{
    return (isOnRight ? " . " : "") + toString();
}

bool SelfEvaluatingValue::isSelfEvaluating() const
{
    return true;
}

ValuePtr ProcValue::call(const vector<ValuePtr>& args)
{
    if (!isValidParamCnt(args))
    {
        string errorInfo = "Params count should be ";
        if (_minParamCnt != UNLIMITED)
            errorInfo += ">= " + to_string(_minParamCnt) + " and ";
        if (_maxParamCnt != UNLIMITED)
            errorInfo += "<= " + to_string(_maxParamCnt);
        errorInfo += " but " + to_string(args.size()) + " was(were) given.";
        throw LispError(errorInfo);
    }
    return proc(args);
}

bool ProcValue::isCallable() const
{
    return true;
}

bool ProcValue::isValidParamCnt(const vector<ValuePtr>& params)
{
    return (_minParamCnt == UNLIMITED || params.size() >= _minParamCnt) && (_maxParamCnt == UNLIMITED || params.size() <= _maxParamCnt);
}

string BuiltinProcValue::toString() const
{
    return "#procedure";
}

string BuiltinProcValue::getTypeName() const
{
    return "builtin proc value";
}

bool ListValue::isList() const
{
    return true;
}

bool ListValue::isNil()
{
    return toVector().size() == 0;
}
