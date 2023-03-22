#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <unordered_map>
#include <memory>
#include <stdexcept>

#include "./value.h"
#include "./error.h"

using std::unordered_map, std::make_shared, std::out_of_range;

class EvalEnv
{
    unordered_map<string, ValuePtr> symbolTable;
public:
    ValuePtr eval(ValuePtr expr);
};

#endif
