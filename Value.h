#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <memory>

using std::ostream, std::endl, std::string, std::to_string, std::shared_ptr;

class Value;

using ValuePtr = shared_ptr<Value>;

class Value
{
    friend ostream& operator<<(ostream& os, const Value& thisValue);
    friend class PairValue;
public:
    virtual string toString() const = 0;
    virtual string getTypeName() const = 0; 
protected:
    Value() {}
    virtual string extractString(bool isOnRight) const;
};

class BooleanValue
    :public Value
{
    bool bValue;
public:
    BooleanValue(bool b) 
        :bValue{ b } {}
    string toString() const override;
    string getTypeName() const override;
};

class NumericValue
    :public Value
{
    double dValue;
protected:
    bool isInteger() const;
public:
    NumericValue(double d)
        :dValue{ d } {}
    string toString() const override;
    string getTypeName() const override;
};

class StringValue
    :public Value
{
    string szValue;
public:
    StringValue(const string& s)
        :szValue{ s } {}
    string toString() const override;
    string getTypeName() const override;
};

class NilValue
    :public Value
{
public:
    NilValue() = default;
    string toString() const override;
    string getTypeName() const override;
protected:
    string extractString(bool isOnRight) const override;
};

class SymbolValue
    :public Value
{
    string szSymbolName;
public:
    SymbolValue(const string& name)
        :szSymbolName{ name } {}
    string toString() const override;
    string getTypeName() const override;
};

class PairValue
    :public Value
{
    ValuePtr pLeftValue;
    ValuePtr pRightValue;
public:
    PairValue(ValuePtr pLeft, ValuePtr pRight)
        :pLeftValue{ pLeft }, pRightValue{ pRight } {}
    string toString() const override;
    string getTypeName() const override;
protected:
    string extractString(bool isOnRight) const override;
};

#endif
