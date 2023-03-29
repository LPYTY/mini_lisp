#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "./value.h"
#include "./error.h"
#include "./builtins.h"

using std::unordered_map, std::make_shared, std::out_of_range, std::enable_shared_from_this;

using EnvPtr = shared_ptr<EvalEnv>;

class EvalEnv
    :public enable_shared_from_this<EvalEnv>
{
    EnvPtr pParent;
    unordered_map<string, ValuePtr> symbolTable;
    EvalEnv(EnvPtr parent);
public:
    static EnvPtr createGlobal();
    static EnvPtr createChild(EnvPtr parent, vector<string> names = {}, ValueList values = {});
    ValuePtr findValue(const string& name);
    void defineValue(const string& name, ValuePtr value);
    ValuePtr eval(ValuePtr expr);
    ValueList evalParams(ValuePtr list);
    ValuePtr apply(ValuePtr proc, const ValueList& params);
};

EvalEnv& getCurrentEvalEnv(); // Unfinished

#endif
