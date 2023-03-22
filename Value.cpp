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

bool PairValue::isList() const
{
    return true;
}

vector<ValuePtr> PairValue::toVector()
{
    vector<ValuePtr> leftVector = pLeftValue->toVector(), rightVector = pRightValue->toVector();
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

bool Value::isNil() const
{
    return false;
}

bool Value::isList() const
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

string Value::extractString(bool isOnRight) const
{
    return (isOnRight ? " . " : "") + toString();
}

bool SelfEvaluatingValue::isSelfEvaluating() const
{
    return true;
}
