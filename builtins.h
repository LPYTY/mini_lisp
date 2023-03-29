#ifndef BUILTINS_H
#define BUILTINS_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cmath>
#include <complex>

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

        double intNearZero(double x);
    }
    using namespace Builtin::Helper;

    namespace Core
    {
        ValuePtr print(const ValueList& params);
        ValuePtr display(const ValueList& params);
        ValuePtr exit(const ValueList& params);
        ValuePtr newline(const ValueList& params);
    }

    namespace TypeCheck
    {
        template<int typeID>
        ValuePtr isType(const ValueList& params)
        {
            return make_shared<BooleanValue>(params[0]->isType(typeID));
        }
        ValuePtr isInteger(const ValueList& params);
        ValuePtr isList(const ValueList& params);
    }

    namespace ListOperator
    {
        ValuePtr append(const ValueList& params);
        ValuePtr car(const ValueList& params);
        ValuePtr cdr(const ValueList& params);
        ValuePtr cons(const ValueList& params);
        ValuePtr length(const ValueList& params);
        ValuePtr list(const ValueList& params);
    }

    namespace Math
    {
        ValuePtr add(const ValueList& params);
        ValuePtr minus(const ValueList& params);
        ValuePtr multiply(const ValueList& params);
        ValuePtr divide(const ValueList& params);
        ValuePtr abs(const ValueList& params);
        ValuePtr expt(const ValueList& params);
        ValuePtr quotient(const ValueList& params);
        ValuePtr remainder(const ValueList& params);
        ValuePtr modulo(const ValueList& params);
    }

    namespace Compare
    {
        ValuePtr equal(const ValueList& params);
        ValuePtr less(const ValueList& params);
        ValuePtr more(const ValueList& params);
        ValuePtr lessOrEqual(const ValueList& params);
        ValuePtr moreOrEqual(const ValueList& params);
        ValuePtr isEven(const ValueList& params);
        ValuePtr isOdd(const ValueList& params);
        ValuePtr isZero(const ValueList& params);
    }
}


extern unordered_map<string, ProcPtr> allBuiltins;

#endif // !BUILTINS_H

