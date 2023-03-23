#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <stdexcept>
#include <optional>

#include "./error.h"

using std::ostream, std::endl, std::string, std::to_string, std::shared_ptr, std::vector, 
std::deque, std::out_of_range, std::enable_shared_from_this, std::optional, std::nullopt;

class Value;

using ValuePtr = shared_ptr<Value>;
using ReadOnlyValuePtr = shared_ptr<const Value>;

class Value
    :public enable_shared_from_this<Value>
{
    friend ostream& operator<<(ostream& os, const Value& thisValue);
    friend class PairValue;
public:
    virtual string toString() const = 0;
    virtual string getTypeName() const = 0; 
    virtual bool isSelfEvaluating() const;
    virtual bool isNil();
    virtual bool isList() const;
    virtual bool isCallable() const;
    virtual vector<ValuePtr> toVector();
    virtual optional<string> asSymbol() const;
    virtual optional<double> asNumber() const;
protected:
    Value() {}
    virtual string extractString(bool isOnRight) const;
    virtual ~Value() = default;
};

class SelfEvaluatingValue
    :public Value
{
protected:
    SelfEvaluatingValue() {}
public:
    bool isSelfEvaluating() const override;
};

class BooleanValue
    :public SelfEvaluatingValue
{
    bool bValue;
public:
    BooleanValue(bool b) 
        :bValue{ b } {}
    string toString() const override;
    string getTypeName() const override;
};

class NumericValue
    :public SelfEvaluatingValue
{
    double dValue;
protected:
    bool isInteger() const;
public:
    NumericValue(double d)
        :dValue{ d } {}
    string toString() const override;
    string getTypeName() const override;
    optional<double> asNumber() const override;
};

class StringValue
    :public SelfEvaluatingValue
{
    string szValue;
public:
    StringValue(const string& s)
        :szValue{ s } {}
    string toString() const override;
    string getTypeName() const override;
};

class ListValue
    :public Value
{
public:
    bool isList() const override;
    bool isNil() override;
};
class NilValue
    :public ListValue
{
public:
    NilValue() = default;
    string toString() const override;
    string getTypeName() const override;
    vector<ValuePtr> toVector() override;
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
    optional<string> asSymbol() const override;
};

class PairValue
    :public ListValue
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
    static shared_ptr<PairValue> fromVector(vector<ValuePtr>& v);
    static shared_ptr<PairValue> fromDeque(deque<ValuePtr>& q);
    string toString() const override;
    string getTypeName() const override;
    vector<ValuePtr> toVector() override;
protected:
    string extractString(bool isOnRight) const override;
};

using FuncType = ValuePtr(const vector<ValuePtr>&);
using FuncPtr = FuncType*;

class ProcValue
    :public SelfEvaluatingValue
{
    FuncPtr proc;
    int _minParamCnt;
    int _maxParamCnt;
public:
    const static int UNLIMITED = -1;
    ProcValue(FuncType procedure, int minArgs = UNLIMITED, int maxArgs = UNLIMITED)
        :proc(procedure), _minParamCnt(minArgs), _maxParamCnt(maxArgs) {}
    virtual ValuePtr call(const vector<ValuePtr>& args);
    bool isCallable() const override;
protected:
    bool isValidParamCnt(const vector<ValuePtr>& params);
};

using ProcPtr = shared_ptr<ProcValue>;

class BuiltinProcValue
    :public ProcValue
{
public:
    BuiltinProcValue(FuncType procedure, int minArgs = UNLIMITED, int maxArgs = UNLIMITED)
        :ProcValue(procedure, minArgs, maxArgs) {}
    string toString() const override;
    string getTypeName() const override;
};
#endif

