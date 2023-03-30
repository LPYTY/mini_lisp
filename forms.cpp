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
        ValuePtr lambdaForm(const ValueList& params, EvalEnv& env)
        {
            auto paramList = params[0]->toVector();
            vector<string> paramNames;
            for (auto& param : paramList)
            {
                if (auto name = param->asSymbol())
                    paramNames.push_back(*name);
                else
                    throw LispError("Expect symbol in Lambda parameter, found " + param->toString());
            }
            vector<ValuePtr> body(params.begin() + 1, params.end());
            return make_shared<LambdaValue>(paramNames, body);
        }

        ValuePtr defineForm(const ValueList& params, EvalEnv& env)
        {
            if (auto name = params[0]->asSymbol())
            {
                env.defineVariable(*name, env.eval(params[1]));
                return make_shared<NilValue>();
            }
            else if (params[0]->isType(ValueType::ListType) && static_pointer_cast<ListValue>(params[0])->isList())
            {
                auto procSymbol = static_pointer_cast<PairValue>(params[0])->left();
                auto name = procSymbol->asSymbol();
                if (!name)
                    throw LispError("In lambda definition, " + procSymbol->toString() + " is not a symbol name");
                ValueList args({ static_pointer_cast<PairValue>(params[0])->right() });
                args.insert(args.end(), params.begin() + 1, params.end());
                env.defineVariable(*name, lambdaForm(args, env));
                return make_shared<SymbolValue>(*name);
            }
            else
            {
                throw LispError("Malformed define form: " + params[0]->toString());
            }
        }

        ValuePtr quoteForm(const ValueList& params, EvalEnv& env)
        {
            return params[0];
        }

        ValuePtr ifForm(const ValueList& params, EvalEnv& env)
        {
            if (*env.eval(params[0]))
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
    SpecialFormItem("lambda"s, SpecialForm::Primary::lambdaForm, 2, ProcValue::UnlimitedCnt, {ValueType::ListType}),
    SpecialFormItem("define"s, SpecialForm::Primary::defineForm, 2, 2, {ValueType::SymbolType, ValueType::AllType}),
    SpecialFormItem("quote"s, SpecialForm::Primary::quoteForm, 1, 1),
    SpecialFormItem("if"s, SpecialForm::Primary::ifForm, 2, 3),
    SpecialFormItem("and"s, SpecialForm::Primary::andForm),
    SpecialFormItem("or"s, SpecialForm::Primary::orForm),
};

