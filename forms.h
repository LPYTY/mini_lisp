#ifndef FORMS_H
#define FORMS_H

#include <unordered_map>
#include <string>

#include "./value.h"
#include "./error.h"

using std::string, std::unordered_map, std::pair;

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
    }

    using namespace ::SpecialForm::Helper;

    namespace Primary
    {
        ValuePtr lambdaForm(const ValueList& params, EvalEnv& env);
        ValuePtr defineForm(const ValueList& params, EvalEnv& env);
        ValuePtr quoteForm(const ValueList& params, EvalEnv& env);
        ValuePtr ifForm(const ValueList& params, EvalEnv& env);
        ValuePtr andForm(const ValueList& params, EvalEnv& env);
        ValuePtr orForm(const ValueList& params, EvalEnv& env);
    }
    
    namespace Derived
    {

    }
}

extern const unordered_map<string, ProcPtr> allSpecialForms;

#endif // !FORMS_H

