#include "forms.h"
#include "eval_env.h"

namespace SpecialForm
{
    namespace Helper
    {
        pair<string, shared_ptr<SpecialFormValue>> SpecialFormItem(string name, FuncType func, int minArgs, int maxArgs, const vector<int>& paramType)
        {
            return make_pair(name, make_shared <SpecialFormValue> (func, minArgs, maxArgs, paramType));
        }
    }

    namespace Primary
    {
        ValuePtr define(const ValueList& params, EvalEnv& env)
        {
            if (auto name = env.eval(params[0])->asSymbol())
            {
                env.defineVariable(*name, env.eval(params[1]));
                return make_shared<NilValue>();
            }
            else
            {
                throw LispError("Malformed define form: " + params[0]->toString());
            }
        }

        ValuePtr quote(const ValueList& params, EvalEnv& env)
        {
            return params[0];
        }

        ValuePtr ifForm(const ValueList& params, EvalEnv& env)
        {
            if (*params[0])
                return env.eval(params[1]);
            else
                return params.size() >= 3 ? env.eval(params[2]) : make_shared<NilValue>();
        }

        ValuePtr andForm(const ValueList& params, EvalEnv& env)
        {
            for (auto& value : params)
            {
                if (!*env.eval(value))
                    return make_shared<BooleanValue>(false);
            }
            return make_shared<BooleanValue>(true);
        }

        ValuePtr orForm(const ValueList& params, EvalEnv& env)
        {
            for (auto& value : params)
            {
                if (*env.eval(value))
                    return make_shared<BooleanValue>(true);
            }
            return make_shared<BooleanValue>(false);
        }
    }
    
}

using namespace SpecialForm::Helper;
using namespace std::literals;

const unordered_map<string, ProcPtr> allSpecialForms =
{
    SpecialFormItem("define"s, SpecialForm::Primary::define, 2, 2, {ValueType::SymbolType, ValueType::AllType}),
    SpecialFormItem("quote"s, SpecialForm::Primary::quote, 1, 1),
    SpecialFormItem("if"s, SpecialForm::Primary::ifForm, 2, 3),
    SpecialFormItem("and"s, SpecialForm::Primary::andForm),
    SpecialFormItem("or"s, SpecialForm::Primary::orForm),
};

