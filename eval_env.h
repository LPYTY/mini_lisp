#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "./value.h"
#include "./error.h"
#include "./builtins.h"

using std::unordered_map, std::make_shared, std::out_of_range;

class EvalEnv
{
    unordered_map<string, ValuePtr> symbolTable;
public:
    EvalEnv();
    ValuePtr eval(ValuePtr expr);
    vector<ValuePtr> evalParams(ValuePtr list);
    ValuePtr apply(ValuePtr proc, const vector<ValuePtr>& params);
};

#endif
