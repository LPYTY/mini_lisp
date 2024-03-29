#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <stdexcept>
#include <optional>
#include <functional>

#include "./error.h"

using std::ostream, std::endl, std::string, std::to_string, std::shared_ptr, std::vector,
std::deque, std::out_of_range, std::enable_shared_from_this, std::optional, std::nullopt,
std::make_shared;

class EvalEnv; // Defined in eval_env.h
using EnvPtr = shared_ptr<EvalEnv>;

namespace ValueType
{
    constexpr int BooleanType        = 0b0000000000000001;
    constexpr int NumericType        = 0b0000000000000010;
    constexpr int StringType         = 0b0000000000000100;
    constexpr int NilType            = 0b0000000000001000;
    constexpr int SymbolType         = 0b0000000000010000;
    constexpr int PairType           = 0b0000000000100000;
    constexpr int BuiltinProcType    = 0b0000000001000000;
    constexpr int SpecialFormType    = 0b0000000010000000;
    constexpr int LambdaType         = 0b0000000100000000;
    constexpr int PromiseType        = 0b0000001000000000;
    constexpr int CharType           = 0b0000010000000000;
    constexpr int VectorType         = 0b0000100000000000;
    constexpr int SelfEvaluatingType = BooleanType | NumericType | StringType | BuiltinProcType | SpecialFormType | LambdaType | PromiseType | CharType;
    constexpr int ListType           = NilType | PairType;
    constexpr int AtomType           = BooleanType | NumericType | StringType | SymbolType | NilType | CharType;
    constexpr int CallableType       = BuiltinProcType | SpecialFormType | LambdaType;
    constexpr int ProcedureType      = BuiltinProcType | LambdaType;
    constexpr int AllType            = BooleanType | NumericType | StringType | NilType | SymbolType | PairType | BuiltinProcType | SpecialFormType | LambdaType | PromiseType | CharType | VectorType;

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
    virtual string toDisplayString() const;
    virtual int getTypeID() const = 0; 
    virtual bool isType(int typeID) const;
    virtual ValueList toVector();
    virtual optional<string> asSymbol() const;
    virtual optional<double> asNumber() const;
    explicit virtual operator bool();
    virtual ValuePtr copy() const = 0;
protected:
    Value() {}
    virtual string extractString(bool isOnRight) const;
    virtual string extractDisplayString(bool isOnRight) const;
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
    explicit operator bool() override;
    ValuePtr copy() const override;
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
    ValuePtr copy() const override;
};

class CharValue
    :public Value
{
    char cValue;
public:
    CharValue(char value)
        :cValue{ value } {}
    string toString() const override;
    string toDisplayString() const override;
    int getTypeID() const override;
    char value() const;
    ValuePtr copy() const override;
};

class StringValue
    :public Value
{
    string szValue;
    static string escChars;
public:
    StringValue(const string& s)
        :szValue{ s } {}
    string toString() const override;
    string toDisplayString() const override;
    int getTypeID() const override;
    string& value();
    const string& value() const;
    char& at(long long index);
    ValuePtr copy() const override;
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
    ValuePtr copy() const override;
protected:
    string extractString(bool isOnRight) const override;
    string extractDisplayString(bool isOnRight) const override;
};

class VectorValue
    :public Value
{
    ValueList vecValue;
public:
    VectorValue(const ValueList& value)
        :vecValue(value) {}
    VectorValue(ValueList&& value)
        :vecValue(value) {}
    string toString() const override;
    string toDisplayString() const override;
    int getTypeID() const override;
    ValueList& value();
    ValuePtr& at(long long index);
    ValuePtr copy() const override;
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
    ValuePtr copy() const override;
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
    string toDisplayString() const override;
    int getTypeID() const override;
    ValueList toVector() override;
    ValuePtr left();
    ValuePtr right();
    bool isList() override;
    ValuePtr copy() const override;
protected:
    string extractString(bool isOnRight) const override;
    string extractDisplayString(bool isOnRight) const override;
};

template<typename Iter>
shared_ptr<ListValue> createListFromIter(Iter begin, Iter end)
{
    if (begin == end)
        return make_shared<NilValue>();
    ValuePtr left = *begin;
    return make_shared<PairValue>(left, createListFromIter(++begin, end));
}

using FuncType = std::function<ValuePtr(const ValueList&, EvalEnv&)>;
using FuncPtr = FuncType*;

/*
class ParamChecker
{
    int minCount;
    int maxCount;
    vector<int> paramType;
public:

    const static int UnlimitedCnt = -1;
    const static int SameToRest = -1;
    const static vector<int> UnlimitedType;

    ParamChecker(int minCount, int maxCount, const vector<int>& paramType)
        :minCount{ minCount }, maxCount{ maxCount }, paramType{ paramType } {}
    bool isValid(const ValueList& params);
};
*/

class CallableValue
    :public Value
{
protected:
    FuncType proc;
    int minParamCnt;
    int maxParamCnt;
    vector<int> paramType;
public:
    const static int UnlimitedCnt = -1;
    const static int SameToRest = 0;
    const static vector<int> UnlimitedType;
    CallableValue(FuncType procedure, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt, vector<int> type = UnlimitedType)
        :proc(procedure), minParamCnt(minArgs), maxParamCnt(maxArgs), paramType(type) {}
    virtual ValuePtr call(const ValueList& args, EvalEnv& env);
    static void assertParamCnt(const ValueList& params, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt);
protected:
    virtual void checkValidParamCnt(const ValueList& params);
    virtual void checkValidParamType(const ValueList& params);
};

using CallablePtr = shared_ptr<CallableValue>;

class ProcValue
    :public CallableValue
{
public:
    using CallableValue::CallableValue;
    string toString() const override;
};
class BuiltinProcValue
    :public ProcValue
{
public:
    using ProcValue::ProcValue;
    int getTypeID() const override;
    static void assertParamCnt(const ValueList& params, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt);
    ValuePtr copy() const override;
protected:
    virtual void checkValidParamCnt(const ValueList& params) override;
};

class LambdaValue
    :public ProcValue
{
    vector<string> paramNames;
    ValueList body;
    EnvPtr parentEnv;
    static ValuePtr standardLambdaProc(const ValueList& params, EvalEnv& env);
public:
    LambdaValue(const vector<string>& paramsDefinition, const ValueList& bodyDefinition, EnvPtr parentEvalEnv);
    int getTypeID() const override;
    ValuePtr call(const ValueList& params, EvalEnv& env) override;
    static void assertParamCnt(const ValueList& params, int argCnt = UnlimitedCnt);
    ValuePtr copy() const override;
protected:
    virtual void checkValidParamCnt(const ValueList& params) override;
    EnvPtr prepareEvalEnv(const ValueList& params);
};

class SpecialFormValue
    :public CallableValue
{
public:
    using CallableValue::CallableValue;
    int getTypeID() const override;
    string toString() const override;
    static void assertParamCnt(const ValueList& params, int minArgs = UnlimitedCnt, int maxArgs = UnlimitedCnt);
    ValuePtr copy() const override;
protected:
    virtual void checkValidParamCnt(const ValueList& params) override;
};

using FormPtr = shared_ptr<SpecialFormValue>;

class PromiseValue
    :public Value
{
    ValuePtr value;
    bool isEvaluated;
public:
    PromiseValue(ValuePtr value)
        :value{ value }, isEvaluated{ false } {}
    int getTypeID() const override;
    string toString() const override;
    ValuePtr force(EvalEnv& env);
    ValuePtr copy() const override;
};

#endif

