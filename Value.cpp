#include "Value.h"

ostream& operator<<(ostream& os, const Value& thisValue)
{
    os << thisValue.toString() << endl;
    return os;
}

string BooleanValue::toString() const
{
    return bValue ? "#t" : "#f";
}

string BooleanValue::type() const
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

string NumericValue::type() const
{
    return "NUMERIC VALUE";
}

string StringValue::toString() const
{
    return '"' + szValue + '"';
}

string StringValue::type() const
{
    return "STRING VALUE";
}

string NilValue::toString() const
{
    return "()";
}

string NilValue::type() const
{
    return "NIL VALUE";
}

ValueList NilValue::toList() const
{
    return ValueList();
}

string SymbolValue::toString() const
{
    return szSymbolName;
}

string SymbolValue::type() const
{
    return "SYMBOL VALUE";
}

string PairValue::toString() const
{
    string ret = "(";
    ValueList vList = toList();
    size_t len = vList.size();
    for (size_t i = 0; i < len; i++)
    {
        ret += vList[i]->toString();
        if (i + 1 < len)
            ret += ' ';
    }
    ret += ')';
    return ret;
}

string PairValue::type() const
{
    return "PAIR VALUE";
}

ValueList PairValue::toList() const
{
    ValueList leftList = pLeftValue->toList(),
        rightList = pRightValue->toList();
    leftList.insert(leftList.end(), rightList.begin(), rightList.end());
    return leftList;
}

ValueList Value::toList() const
{
    ValueList ret;
    ret.push_back(shared_from_this());
    return ret;
}
