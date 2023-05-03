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
            int minArgs = CallableValue::UnlimitedCnt,
            int maxArgs = CallableValue::UnlimitedCnt,
            const vector<int>& paramType = CallableValue::UnlimitedType
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
        ValuePtr delayForm(const ValueList& params, EvalEnv& env);
    }
}

extern const unordered_map<string, FormPtr> allSpecialForms;

#endif // !FORMS_H

