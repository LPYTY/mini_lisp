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
            return make_shared<LambdaValue>(paramNames, body, env.shared_from_this());
        }

        ValuePtr defineForm(const ValueList& params, EvalEnv& env)
        {
            if (auto name = params[0]->asSymbol())
            {
                SpecialFormValue::assertParamCnt(params, 2, 2);
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
    }

    namespace Derived
    {
        using namespace Primary;
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

        ValuePtr condForm(const ValueList& params, EvalEnv& env)
        {
            ValuePtr result = make_shared<NilValue>();
            auto condEnv = EvalEnv::createChild(env.shared_from_this(), { "else" }, { make_shared<BooleanValue>(true) });
            auto currentEnv = *condEnv;

            for (size_t i = 0; i < params.size(); i++)
            {
                auto subList = params[i]->toVector();
                SpecialFormValue::assertParamCnt(subList, 1);
                if (subList[0]->asSymbol() == "else" && i != params.size() - 1)
                    throw LispError("else clause must be the last one.");
            }

            for (size_t i = 0; i < params.size(); i++)
            {
                auto subList = params[i]->toVector();
                if (subList.size() == 1)
                    result = currentEnv.eval(subList[0]);
                else
                {
                    if (env.eval(subList[0]))
                    {
                        for (size_t i = 1; i < subList.size(); i++)
                            result = currentEnv.eval(subList[i]);
                    }
                }
            }
            return result;
        }

        ValuePtr beginForm(const ValueList& params, EvalEnv& env)
        {
            ValuePtr result;
            for (auto& expr : params)
                result = env.eval(expr);
            return result;
        }

        ValuePtr letForm(const ValueList& params, EvalEnv& env)
        {
            auto subEnv = EvalEnv::createChild(env.shared_from_this());
            auto& currentEnv = *subEnv;
            auto definitions = params[0]->toVector();
            for (auto& definition : definitions)
            {
                auto defineList = definition->toVector();
                defineForm(defineList, currentEnv);
            }
            ValuePtr result = make_shared<NilValue>();
            for (size_t i = 1; i < params.size(); i++)
            {
                result = currentEnv.eval(params[i]);
            }
            return result;
        }

        ValuePtr quasiquoteForm(const ValueList& params, EvalEnv& env)
        {
            auto quasiquoteEnv = EvalEnv::createChild(
                env.shared_from_this(), 
                { "unquote" }, 
                { make_shared<SpecialFormValue>(unquoteForm, 1, 1) }
            );
            auto& currentEnv = *quasiquoteEnv;
            if (!params[0]->isType(ValueType::PairType))
                return params[0];
            auto valueList = static_pointer_cast<PairValue>(params[0]);
            if (valueList->left()->asSymbol() == "unquote")
            {
                return currentEnv.eval(params[0]);
            }
            if (!valueList->isList())
                return params[0];
            auto values = valueList->toVector();
            ValueList result;
            std::ranges::transform(
                values,
                std::back_inserter(result),
                [&currentEnv](ValuePtr param)
                {
                    return quasiquoteForm({ param }, currentEnv);
                }
            );
            return ListValue::fromVector(result);
        }

        ValuePtr unquoteForm(const ValueList& params, EvalEnv& env)
        {
            return env.eval(params[0]);
        }
    }
}

using namespace SpecialForm::Helper;
using namespace std::literals;

const unordered_map<string, ProcPtr> allSpecialForms =
{
    SpecialFormItem("lambda"s, SpecialForm::Primary::lambdaForm, 2, ProcValue::UnlimitedCnt, {ValueType::ListType}),
    SpecialFormItem("define"s, SpecialForm::Primary::defineForm, 2, ProcValue::UnlimitedCnt, {ValueType::SymbolType, ValueType::AllType}),
    SpecialFormItem("quote"s, SpecialForm::Primary::quoteForm, 1, 1),
    SpecialFormItem("if"s, SpecialForm::Primary::ifForm, 2, 3),
    SpecialFormItem("cond"s, SpecialForm::Derived::condForm, ProcValue::UnlimitedCnt, ProcValue::UnlimitedCnt, {ValueType::ListType, ProcValue::SameToRest}),
    SpecialFormItem("let"s, SpecialForm::Derived::letForm, 2, ProcValue::UnlimitedCnt, {ValueType::ListType}),
    SpecialFormItem("begin"s, SpecialForm::Derived::beginForm, 1),
    SpecialFormItem("and"s, SpecialForm::Derived::andForm),
    SpecialFormItem("or"s, SpecialForm::Derived::orForm),
    SpecialFormItem("quasiquote"s, SpecialForm::Derived::quasiquoteForm, 1, 1),
};

