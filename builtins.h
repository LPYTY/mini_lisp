#ifndef BUILTINS_H
#define BUILTINS_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <complex>
#include <functional>
#include <ranges>
#include <algorithm>

#include "./value.h"

using std::cout, std::vector, std::to_string, std::make_shared, std::unordered_map, std::pair, std::make_pair, std::static_pointer_cast, std::function;

class EvalEnv;

namespace Builtin
{
    using namespace std::placeholders;

    using BuiltinFunc = function<ValuePtr(const ValueList&, EvalEnv& env)>;

    namespace Helper // Not in builtin functions list
    {
        pair<string, shared_ptr<BuiltinProcValue>> BuiltinItem(
            string name,
            FuncType func,
            int minArgs = CallableValue::UnlimitedCnt,
            int maxArgs = CallableValue::UnlimitedCnt,
            const vector<int>& paramType = CallableValue::UnlimitedType
        );

        template<typename T>
        struct compare
        {
            ValuePtr operator()(const ValueList& params, const function<bool(const T&, const T&)>& Comp, const function<T(ValuePtr)>& Conv)
            {
                return make_shared<BooleanValue>(Comp(Conv(params[0]), Conv(params[1])));
            }
        };

        template<typename T>
        struct isEqual
        {
            bool operator()(const T& lhs, const T& rhs)
            {
                return lhs == rhs;
            }
        };

        double numberConv(ValuePtr value);
        string stringConv(ValuePtr value);
        string stringCiConv(ValuePtr value);

        string ci(const string& s);
    }
    using namespace Builtin::Helper;

    namespace Core
    {
        ValuePtr apply(const ValueList& params, EvalEnv& env);
        ValuePtr print(const ValueList& params, EvalEnv& env);
        ValuePtr display(const ValueList& params, EvalEnv& env);
        ValuePtr displayln(const ValueList& params, EvalEnv& env);
        ValuePtr error(const ValueList& params, EvalEnv& env);
        ValuePtr eval(const ValueList& params, EvalEnv& env);
        ValuePtr exit(const ValueList& params, EvalEnv& env);
        ValuePtr newline(const ValueList& params, EvalEnv& env);
    }

    namespace TypeCheck
    {
        template<int typeID>
        ValuePtr isType(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(params[0]->isType(typeID));
        }
        ValuePtr isInteger(const ValueList& params, EvalEnv& env);
        ValuePtr isList(const ValueList& params, EvalEnv& env);
    }

    namespace ListOperator
    {
        ValuePtr append(const ValueList& params, EvalEnv& env);
        ValuePtr car(const ValueList& params, EvalEnv& env);
        ValuePtr cdr(const ValueList& params, EvalEnv& env);
        ValuePtr cons(const ValueList& params, EvalEnv& env);
        ValuePtr length(const ValueList& params, EvalEnv& env);
        ValuePtr list(const ValueList& params, EvalEnv& env);
        ValuePtr map(const ValueList& params, EvalEnv& env);
        ValuePtr filter(const ValueList& params, EvalEnv& env);
        ValuePtr reduce(const ValueList& params, EvalEnv& e);
    }

    namespace Math
    {
        ValuePtr add(const ValueList& params, EvalEnv& e);
        ValuePtr minus(const ValueList& params, EvalEnv& e);
        ValuePtr multiply(const ValueList& params, EvalEnv& e);
        ValuePtr divide(const ValueList& params, EvalEnv& e);
        ValuePtr abs(const ValueList& params, EvalEnv& e);
        ValuePtr expt(const ValueList& params, EvalEnv& e);
        ValuePtr quotient(const ValueList& params, EvalEnv& e);
        ValuePtr remainder(const ValueList& params, EvalEnv& e);
        ValuePtr modulo(const ValueList& params, EvalEnv& e);
    }

    namespace Compare
    {
        ValuePtr eq(const ValueList& params, EvalEnv& e);
        ValuePtr equal(const ValueList& params, EvalEnv& e);
        ValuePtr _not(const ValueList& params, EvalEnv& e);
        extern BuiltinFunc less;
        extern BuiltinFunc more;
        extern BuiltinFunc lessOrEqual;
        extern BuiltinFunc moreOrEqual;
        ValuePtr isEven(const ValueList& params, EvalEnv& e);
        ValuePtr isOdd(const ValueList& params, EvalEnv& e);
        ValuePtr isZero(const ValueList& params, EvalEnv& e);
    }

    namespace String
    {
        ValuePtr makeString(const ValueList& params, EvalEnv& env);
        ValuePtr _string(const ValueList& params, EvalEnv& env);
        ValuePtr stringLength(const ValueList& params, EvalEnv& env);
        ValuePtr stringRef(const ValueList& params, EvalEnv& env);
        ValuePtr stringSet(const ValueList& params, EvalEnv& env);
        extern BuiltinFunc stringEqual;
        extern BuiltinFunc stringEqualCi;
        extern BuiltinFunc stringGreater;
        extern BuiltinFunc stringSmaller;
        extern BuiltinFunc stringGreaterOrEqual;
        extern BuiltinFunc stringSmallerOrEqual;
        extern BuiltinFunc stringGreaterCi;
        extern BuiltinFunc stringSmallerCi;
        extern BuiltinFunc stringGreaterOrEqualCi;
        extern BuiltinFunc stringSmallerOrEqualCi;
        ValuePtr subString(const ValueList& params, EvalEnv& env);
        ValuePtr stringAppend(const ValueList& params, EvalEnv& env);
        ValuePtr listToString(const ValueList& params, EvalEnv& env);
        ValuePtr stringToList(const ValueList& params, EvalEnv& env);
        ValuePtr stringCopy(const ValueList& params, EvalEnv& env);
        ValuePtr stringFill(const ValueList& params, EvalEnv& env);
    }

    namespace Control
    {
        ValuePtr force(const ValueList& params, EvalEnv& env);
    }
}

extern unordered_map<string, CallablePtr> allBuiltins;

#endif // !BUILTINS_H
