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
std::deque, std::out_of_range, std::enable_shared_from_this, std::optional, std::nullopt,
std::make_shared;

namespace ValueType
{
    const int BooleanType        = 0x0000000001;
    const int NumericType        = 0x0000000010;
    const int StringType         = 0x0000000100;
    const int NilType            = 0x0000001000;
    const int SymbolType         = 0x0000010000;
    const int PairType           = 0x0000100000;
    const int BuiltinProcType    = 0x0001000000;
    const int SelfEvaluatingType = BooleanType | NumericType | StringType | BuiltinProcType;
    const int ListType = NilType | PairType;
    const int AtomType = BooleanType | NumericType | StringType | SymbolType | NilType;
    const int ProcedureType = BuiltinProcType;
    const int AllType = BooleanType | NumericType | StringType | NilType | SymbolType | PairType | BuiltinProcType;

    string typeName(int typeID);
};

class Value;

using ValuePtr = shared_ptr<Value>;
using ReadOnlyValuePtr = shared_ptr<const Value>;
using ValueList = vector<ValuePtr>;

class Value
    :public enable_shared_from_this<Value>
{
    friend ostream& operator<<(ostream& os, const Value& thisValue);
    friend class PairValue;
public:
    virtual string toString() const = 0;
    virtual int getTypeID() const = 0; 
    virtual bool isType(int typeID) const;
    virtual ValueList toVector();
    virtual optional<string> asSymbol() const;
    virtual optional<double> asNumber() const;
protected:
    Value() {}
    virtual string extractString(bool isOnRight) const;
    virtual ~Value() = default;
};

class BooleanValue
    :public Value
{
    bool bValue;
public:
    BooleanValue(bool b) 
        :bValue{ b } {}
    string toString() const override;
    int getTypeID() const override;
};

class NumericValue
    :public Value
{
    double dValue;
public:
    NumericValue(double d)
        :dValue{ d } {}
    string toString() const override;
    int getTypeID() const override;
    bool isInteger() const;
    optional<double> asNumber() const override;
};

class StringValue
    :public Value
{
    string szValue;
public:
    StringValue(const string& s)
        :szValue{ s } {}
    string toString() const override;
    int getTypeID() const override;
    const string& value() const;
};

class ListValue
    :public Value
{
public:
    virtual bool isEmpty();
    virtual bool isList() = 0;
    static shared_ptr<ListValue> fromVector(const ValueList& v);
    static shared_ptr<ListValue> fromDeque(deque<ValuePtr>& q);
};

class NilValue
    :public ListValue
{
public:
    NilValue() = default;
    string toString() const override;
    int getTypeID() const override;
    ValueList toVector() override;
    bool isList() override;
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
    int getTypeID() const override;
    optional<string> asSymbol() const override;
};

class PairValue
    :public ListValue
{
    ValuePtr pLeftValue;
    ValuePtr pRightValue;
public:
    PairValue(ValuePtr pLeft, ValuePtr pRight)
        :pLeftValue{ pLeft }, pRightValue{ pRight } {}
    string toString() const override;
    int getTypeID() const override;
    ValueList toVector() override;
    ValuePtr left();
    ValuePtr right();
    bool isList() override;
protected:
    string extractString(bool isOnRight) const override;
};

template<typename Iter>
shared_ptr<ListValue> createListFromIter(Iter begin, Iter end)
{
    if (begin == end)
        return make_shared<NilValue>();
    ValuePtr left = *begin;
    return make_shared<PairValue>(left, createListFromIter(++begin, end));
}

using FuncType = ValuePtr(const ValueList&);
using FuncPtr = FuncType*;

class ProcValue
    :public Value
{
    FuncPtr proc;
    int minParamCnt;
    int maxParamCnt;
    vector<int> paramType;
public:
    const static int UnlimitedCnt = -1;
    const static int SameToRest = -1;
    const static vector<int> UnlimitedType;
    ProcValue(FuncType procedure, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt, vector<int> type = UnlimitedType)
        :proc(procedure), minParamCnt(minArgs), maxParamCnt(maxArgs), paramType(type) {}
    virtual ValuePtr call(const ValueList& args);
protected:
    void checkValidParamCnt(const ValueList& params);
    void checkValidParamType(const ValueList& params);
};

using ProcPtr = shared_ptr<ProcValue>;

class BuiltinProcValue
    :public ProcValue
{
public:
    BuiltinProcValue(FuncType procedure, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt, vector<int> type = UnlimitedType)
        :ProcValue(procedure, minArgs, maxArgs, type) {}
    string toString() const override;
    int getTypeID() const override;
};
#endif

