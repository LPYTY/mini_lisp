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
    return "BOOLEAN VALUE";
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
    return "NUMERIC VALUE";
}

string StringValue::toString() const
{
    return '"' + szValue + '"';
}

string StringValue::getTypeName() const
{
    return "STRING VALUE";
}

string NilValue::toString() const
{
    return "()";
}

string NilValue::getTypeName() const
{
    return "NIL VALUE";
}

bool NilValue::isNil() const
{
    return true;
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
    return "SYMBOL VALUE";
}

string PairValue::toString() const
{
    return '(' + extractString(false) + ')';
}

string PairValue::getTypeName() const
{
    return "PAIR VALUE";
}

shared_ptr<PairValue> PairValue::fromVector(vector<ValuePtr>& v)
{
    return fromIter(v.begin(), v.end());
}

shared_ptr<PairValue> PairValue::fromDeque(deque<ValuePtr>& q)
{
    return fromIter(q.begin(), q.end());
}

string PairValue::extractString(bool isOnRight) const
{
    return (isOnRight ? " " : "") + pLeftValue->extractString(false) + pRightValue->extractString(true);
}

bool Value::isSelfEvaluating() const
{
    return false;
}

bool Value::isNil() const
{
    return false;
}

string Value::extractString(bool isOnRight) const
{
    return (isOnRight ? " . " : "") + toString();
}

bool SelfEvaluatingValue::isSelfEvaluating() const
{
    return true;
}
