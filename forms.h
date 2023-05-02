#ifndef FORMS_H
#define FORMS_H

#include <unordered_map>
#include <string>
#include <functional>

#include "./value.h"
#include "./error.h"

using std::string, std::unordered_map, std::pair, std::function;

class EvalEnv;

namespace SpecialForm
{
    namespace Helper
    {
        pair<string, shared_ptr<SpecialFormValue>> SpecialFormItem(
            string name,
            FuncType func,
            int minArgs = ProcValue::UnlimitedCnt,
            int maxArgs = ProcValue::UnlimitedCnt,
            const vector<int>& paramType = ProcValue::UnlimitedType
        );
        bool defineVariable(const ValueList& params, EvalEnv& defineEnv, EvalEnv& evalEnv);
        void defineVariableAndAssert(const ValueList& params, EvalEnv& defineEnv, EvalEnv& evalEnv);
        ValuePtr basicLet(const ValueList& params, EvalEnv& env, function<void(const ValueList&, EvalEnv&, EvalEnv&)> defineOrder);
        void letDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv);
        void letxDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv);
        void letrecDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv);
        //ValuePtr quasiquoteHelper(const ValueList& params, EvalEnv& env, int layerCount);
    }

    using namespace ::SpecialForm::Helper;

    namespace Primary
    {
        ValuePtr lambdaForm(const ValueList& params, EvalEnv& env);
        ValuePtr defineForm(const ValueList& params, EvalEnv& env);
        ValuePtr quoteForm(const ValueList& params, EvalEnv& env);
        ValuePtr ifForm(const ValueList& params, EvalEnv& env);
        ValuePtr setForm(const ValueList& params, EvalEnv& env);
    }
    
    namespace Derived
    {
        ValuePtr condForm(const ValueList& params, EvalEnv& env);
        ValuePtr letForm(const ValueList& params, EvalEnv& env);
        ValuePtr letxForm(const ValueList& params, EvalEnv& env);
        ValuePtr letrecForm(const ValueList& params, EvalEnv& env);
        ValuePtr beginForm(const ValueList& params, EvalEnv& env);
        ValuePtr doForm(const ValueList& params, EvalEnv& env);
        ValuePtr andForm(const ValueList& params, EvalEnv& env);
        ValuePtr orForm(const ValueList& params, EvalEnv& env);
        ValuePtr quasiquoteForm(const ValueList& params, EvalEnv& env);
        ValuePtr unquoteForm(const ValueList& params, EvalEnv& env);
    }
}

extern const unordered_map<string, ProcPtr> allSpecialForms;

#endif // !FORMS_H

