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

        bool defineVariable(const ValueList& params, EvalEnv& defineEnv, EvalEnv& evalEnv)
        {
            if (auto name = params[0]->asSymbol())
            {
                SpecialFormValue::assertParamCnt(params, 2, 2);
                defineEnv.defineVariable(*name, evalEnv.eval(params[1]));
                return true;
            }
        }

        void defineVariableAndAssert(const ValueList& params, EvalEnv& defineEnv, EvalEnv& evalEnv)
        {
            if (!defineVariable(params, defineEnv, evalEnv))
                throw LispError("Malformed define form: " + params[0]->toString());
        }

        ValuePtr basicLet(const ValueList& params, EvalEnv& env, function<void(const ValueList&, EvalEnv&, EvalEnv&)> defineOrder)
        {
            auto subEnv = EvalEnv::createChild(env.shared_from_this());
            auto& currentEnv = *subEnv;
            auto definitions = params[0]->toVector();
            defineOrder(definitions, currentEnv, env);
            ValuePtr result = make_shared<NilValue>();
            for (size_t i = 1; i < params.size(); i++)
            {
                result = currentEnv.eval(params[i]);
            }
            return result;
        }

        void letDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv)
        {
            for (auto& definition : definitions)
            {
                auto defineList = definition->toVector();
                defineVariableAndAssert(defineList, defineEnv, evalEnv);
            }
        }

        void letxDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv)
        {
            for (auto& definition : definitions)
            {
                auto defineList = definition->toVector();
                defineVariableAndAssert(defineList, defineEnv, defineEnv);
            }
        }

        void letrecDefineOrder(const ValueList& definitions, EvalEnv& defineEnv, EvalEnv& evalEnv)
        {
            for (auto& definition : definitions)
            {
                auto defineList = definition->toVector();
                SpecialFormValue::assertParamCnt(defineList, 2, 2);
                defineList[1] = ListValue::fromVector({ make_shared<SymbolValue>("quote"),make_shared<NilValue>() });
                defineVariableAndAssert(defineList, defineEnv, defineEnv);
            }
            letxDefineOrder(definitions, defineEnv, evalEnv);
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
            if (defineVariable(params, env, env))
                return make_shared<NilValue>();
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

        ValuePtr setForm(const ValueList& params, EvalEnv& env)
        {
            auto name = *params[0]->asSymbol();
            auto result = env.findVariable(name);
            if (result.first)
                result.first->defineVariable(name, env.eval(params[1]));
            else
                throw LispError("Variable " + name + " not defined.");
            return make_shared<NilValue>();
        }
    }

    namespace Derived
    {
        using namespace Primary;
        ValuePtr andForm(const ValueList& params, EvalEnv& env)
        {
            ValuePtr result = make_shared<BooleanValue>(true);
            for (auto& value : params)
            {
                result = env.eval(value);
                if (!*result)
                    break;
            }
            return result;
        }

        ValuePtr orForm(const ValueList& params, EvalEnv& env)
        {
            ValuePtr result = make_shared<BooleanValue>(false);
            for (auto& value : params)
            {
                result = env.eval(value);
                if (*result)
                    break;
            }
            return result;
        }

        ValuePtr condForm(const ValueList& params, EvalEnv& env)
        {
            ValuePtr result = make_shared<NilValue>();
            auto condEnv = EvalEnv::createChild(env.shared_from_this(), { "else" }, { make_shared<BooleanValue>(true) });
            auto& currentEnv = *condEnv;

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
                    if (*currentEnv.eval(subList[0]))
                    {
                        for (size_t i = 1; i < subList.size(); i++)
                            result = currentEnv.eval(subList[i]);
                        return result;
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

        ValuePtr doForm(const ValueList& params, EvalEnv& env)
        {
            auto initializers = params[0]->toVector();
            auto testList = params[1]->toVector();
            SpecialFormValue::assertParamCnt(testList, 1);
            auto subEnv = EvalEnv::createChild(env.shared_from_this());
            auto& currentEnv = *subEnv;
            vector<ValueList> initializerLists;
            for (auto& initializer : initializers)
            {
                auto initializerList = initializer->toVector();
                SpecialFormValue::assertParamCnt(initializerList, 2, 3);
                initializerLists.push_back(initializerList);
                ValueList defineParams = { initializerList[0], initializerList[1] };
                defineVariableAndAssert(defineParams, currentEnv, currentEnv);
            }
            auto& test = testList[0];
            while (!*currentEnv.eval(test))
            {
                for (size_t i = 2; i < params.size(); i++)
                    currentEnv.eval(params[i]);
                for (auto& initializerList : initializerLists)
                {
                    if (initializerList.size() == 3)
                    {
                        ValueList defineParams = { initializerList[0], initializerList[2] };
                        defineVariable(defineParams, currentEnv, currentEnv);
                    }
                }
            }
            ValuePtr result = make_shared<NilValue>();
            for (size_t i = 1; i < testList.size(); i++)
                result = currentEnv.eval(testList[i]);
            return result;
        }

        ValuePtr letForm(const ValueList& params, EvalEnv& env)
        {
            if (auto name = params[0]->asSymbol())
            {
                SpecialFormValue::assertParamCnt(params, 3);
                auto subEnv = EvalEnv::createChild(env.shared_from_this());
                auto& currentEnv = *subEnv;
                auto defineLists = params[1]->toVector();
                ValueList variables, bindings, lambdaParams;
                for (auto& definePtr : defineLists)
                {
                    auto defineList = definePtr->toVector();
                    SpecialFormValue::assertParamCnt(defineList, 2);
                    variables.push_back(defineList[0]);
                    bindings.push_back(defineList[1]);
                }
                lambdaParams.push_back(ListValue::fromVector(variables));
                for (size_t i = 2; i < params.size(); i++)
                    lambdaParams.push_back(params[i]);
                auto lambda = lambdaForm(lambdaParams, currentEnv);
                currentEnv.defineVariable(*name, lambda);
                return currentEnv.applyProc(lambda, bindings);
            }
            return basicLet(params, env, letDefineOrder);
        }

        ValuePtr letxForm(const ValueList& params, EvalEnv& env)
        {
            return basicLet(params, env, letxDefineOrder);
        }

        ValuePtr letrecForm(const ValueList& params, EvalEnv& env)
        {
            return basicLet(params, env, letrecDefineOrder);
        }

        ValuePtr quasiquoteForm(const ValueList& params, EvalEnv& env)
        {
            auto quasiquoteEnv = EvalEnv::createChild(
                env.shared_from_this()//, 
                //{ "unquote" }, 
                //{ make_shared<SpecialFormValue>(unquoteForm, 1, 1) }
            );
            auto& currentEnv = *quasiquoteEnv;
            if (!params[0]->isType(ValueType::PairType))
                return params[0];
            auto valueList = static_pointer_cast<PairValue>(params[0]);
            if (valueList->left()->asSymbol() == "unquote")
            {
                //currentEnv.undefVariable("unquote");
                auto unquotedList = valueList->right()->toVector();
                SpecialFormValue::assertParamCnt(unquotedList, 1, 1);
                return env.eval(unquotedList[0]);
                //currentEnv.defineVariable()
            }
            if (!valueList->isList())
                return params[0];
            auto values = valueList->toVector();
            ValueList result;
            for (auto& value : values)
            {
                if (value->isType(ValueType::PairType) && static_pointer_cast<PairValue>(value)->left()->asSymbol() == "unquote-splicing")
                {
                    auto splicingExpressionList = value->toVector();
                    SpecialFormValue::assertParamCnt(splicingExpressionList, 2, 2);
                    auto splicingList = currentEnv.evalParams(splicingExpressionList[1]);
                    result.insert(result.end(), splicingList.begin(), splicingList.end());
                }
                else
                    result.push_back(quasiquoteForm({ value }, currentEnv));
            }
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
    SpecialFormItem("set!"s, SpecialForm::Primary::setForm, 2, 2, {ValueType::SymbolType, ValueType::AllType}),
    SpecialFormItem("cond"s, SpecialForm::Derived::condForm, ProcValue::UnlimitedCnt, ProcValue::UnlimitedCnt, {ValueType::ListType, ProcValue::SameToRest}),
    SpecialFormItem("let"s, SpecialForm::Derived::letForm, 2),
    SpecialFormItem("let*"s, SpecialForm::Derived::letxForm, 2),
    SpecialFormItem("letrec"s, SpecialForm::Derived::letrecForm, 2),
    SpecialFormItem("begin"s, SpecialForm::Derived::beginForm, 1),
    SpecialFormItem("and"s, SpecialForm::Derived::andForm),
    SpecialFormItem("or"s, SpecialForm::Derived::orForm),
    SpecialFormItem("do"s, SpecialForm::Derived::doForm, 2, ProcValue::UnlimitedCnt, {ValueType::ListType, ValueType::ListType}),
    SpecialFormItem("quasiquote"s, SpecialForm::Derived::quasiquoteForm, 1, 1),
};

