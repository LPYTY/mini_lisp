#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <ranges>

#include "./value.h"
#include "./error.h"
#include "./builtins.h"
#include "./forms.h"

using std::unordered_map, std::make_shared, std::out_of_range, std::enable_shared_from_this;

using EnvPtr = shared_ptr<EvalEnv>;

class EvalEnv
    :public enable_shared_from_this<EvalEnv>
{
    EnvPtr pParent;
    unordered_map<string, ProcPtr> specialFormTable;
    unordered_map<string, ValuePtr> symbolTable;
    EvalEnv(EnvPtr parent);
public:
    EvalEnv(const EvalEnv&) = delete;
    EvalEnv& operator=(const EvalEnv&) = delete;
    static EnvPtr createGlobal();
    static EnvPtr createChild(EnvPtr parent, vector<string> names = {}, ValueList values = {});
    pair<EnvPtr, ProcPtr> findForm(const string& name);
    ProcPtr getForm(const string& name);
    pair<EnvPtr, ValuePtr> findVariable(const string& name);
    ValuePtr getVariableValue(const string& name);
    void defineVariable(const string& name, ValuePtr value);
    void undefVariable(const string& name);
    ValuePtr eval(ValuePtr expr);
    ValueList evalParams(const ValueList& list);
    ValueList evalParams(ValuePtr list);
    ValuePtr apply(ValuePtr proc, ValuePtr params);
    ValuePtr applyProc(ValuePtr proc, const ValueList& params);
};


#endif
