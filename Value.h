#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <stdexcept>

using std::ostream, std::endl, std::string, std::to_string, std::shared_ptr, std::vector, std::deque, std::out_of_range;

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
private:
    template<typename Iter>
    static shared_ptr<PairValue> fromIter(Iter begin, Iter end)
    {
        if (begin == end)
            throw out_of_range("Container must have at least 2 items. 0 was given.");
        auto iterForComparation = begin, iterAfterBegin = begin;
        iterForComparation++;
        iterAfterBegin++;
        if (iterForComparation == end)
            throw out_of_range("Container must have at least 2 items. 1 was given.");
        iterForComparation++;
        if (iterForComparation == end) // Container has 2 items
        {
            return make_shared<PairValue>(*begin, *iterAfterBegin);
        }
        // More than 2 items
        return make_shared<PairValue>(*begin, fromIter(iterAfterBegin, end));
    }
public:
    PairValue(ValuePtr pLeft, ValuePtr pRight)
        :pLeftValue{ pLeft }, pRightValue{ pRight } {}
    string toString() const override;
    string getTypeName() const override;
    static shared_ptr<PairValue> fromVector(vector<ValuePtr>& v);
    static shared_ptr<PairValue> fromDeque(deque<ValuePtr>& q);
protected:
    string extractString(bool isOnRight) const override;
};

#endif

