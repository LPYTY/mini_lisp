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

using std::cout, std::vector, std::to_string, std::make_shared, std::unordered_map, std::pair, std::make_pair, std::static_pointer_cast;

class EvalEnv;

namespace Builtin
{
    namespace Helper // Not in builtin functions list
    {
        pair<string, shared_ptr<BuiltinProcValue>> BuiltinItem(
            string name,
            FuncType func,
            int minArgs = ProcValue::UnlimitedCnt,
            int maxArgs = ProcValue::UnlimitedCnt,
            const vector<int>& paramType = ProcValue::UnlimitedType
        );
    }
    using namespace Builtin::Helper;

    namespace Core
    {
        ValuePtr apply(const ValueList& params, EvalEnv& e);
        ValuePtr print(const ValueList& params, EvalEnv& e);
        ValuePtr display(const ValueList& params, EvalEnv& e);
        ValuePtr displayln(const ValueList& params, EvalEnv& e);
        ValuePtr error(const ValueList& params, EvalEnv& e);
        ValuePtr eval(const ValueList& params, EvalEnv& e);
        ValuePtr exit(const ValueList& params, EvalEnv& e);
        ValuePtr newline(const ValueList& params, EvalEnv& e);
    }

    namespace TypeCheck
    {
        template<int typeID>
        ValuePtr isType(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(params[0]->isType(typeID));
        }
        ValuePtr isInteger(const ValueList& params, EvalEnv& e);
        ValuePtr isList(const ValueList& params, EvalEnv& e);
    }

    namespace ListOperator
    {
        ValuePtr append(const ValueList& params, EvalEnv& e);
        ValuePtr car(const ValueList& params, EvalEnv& e);
        ValuePtr cdr(const ValueList& params, EvalEnv& e);
        ValuePtr cons(const ValueList& params, EvalEnv& e);
        ValuePtr length(const ValueList& params, EvalEnv& e);
        ValuePtr list(const ValueList& params, EvalEnv& e);
        ValuePtr map(const ValueList& params, EvalEnv& e);
        ValuePtr filter(const ValueList& params, EvalEnv& e);
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
        ValuePtr less(const ValueList& params, EvalEnv& e);
        ValuePtr more(const ValueList& params, EvalEnv& e);
        ValuePtr lessOrEqual(const ValueList& params, EvalEnv& e);
        ValuePtr moreOrEqual(const ValueList& params, EvalEnv& e);
        ValuePtr isEven(const ValueList& params, EvalEnv& e);
        ValuePtr isOdd(const ValueList& params, EvalEnv& e);
        ValuePtr isZero(const ValueList& params, EvalEnv& e);
    }
}

extern unordered_map<string, ProcPtr> allBuiltins;

#endif // !BUILTINS_H
