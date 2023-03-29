#include "./builtins.h"

using namespace std::literals;
using std::make_pair;

namespace Builtin
{
    namespace Helper
    {
        pair<string, shared_ptr<BuiltinProcValue>> BuiltinItem(string name, FuncType func, int minArgs, int maxArgs, const vector<int>& paramType)
        {
            return make_pair(name, make_shared<BuiltinProcValue>(func, minArgs, maxArgs, paramType));
        }

        double intNearZero(double x)
        {
            if (x == static_cast<long long>(x))
            {
                return x;
            }
            else if (x >= 0)
            {
                return static_cast<long long>(x);
            }
            else
            {
                return static_cast<long long>(x) + 1;
            }
        }
    }

    namespace Core
    {
        ValuePtr print(const ValueList& params, EvalEnv& e)
        {
            for (auto& p : params)
                cout << p->toString() << endl;
            return make_shared<NilValue>();
        }

        ValuePtr display(const ValueList& params, EvalEnv& e)
        {
            for (auto& p : params)
            {
                cout << (p->isType(ValueType::StringType) ? std::static_pointer_cast<StringValue>(p)->value() : p->toString()) << endl;
            }
            return make_shared<NilValue>();
        }

        ValuePtr exit(const ValueList& params, EvalEnv& e)
        {
            int exitCode = 0;
            if (params.size() != 0)
            {
                if (!params[0]->asNumber())
                    throw LispError(params[0]->toString() + " is not a number.");
                exitCode = *params[0]->asNumber();
            }
            throw ExitEvent(exitCode);
        }

        ValuePtr newline(const ValueList& params, EvalEnv& e)
        {
            cout << endl;
            return make_shared<NilValue>();
        }
    }

    namespace TypeCheck
    {
        ValuePtr isInteger(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(params[0]->isType(ValueType::NumericType) && static_pointer_cast<NumericValue>(params[0])->isInteger());
        }

        ValuePtr isList(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(params[0]->isType(ValueType::ListType) && static_pointer_cast<ListValue>(params[0])->isList());
        }
    }

    namespace ListOperator
    {
        ValuePtr append(const ValueList& params, EvalEnv& e)
        {
            if (params.size() == 0)
                return make_shared<NilValue>();
            ValueList resultList;
            for (const auto& p : params)
            {
                if (!p->isType(ValueType::ListType))
                    throw LispError("Expect list, got " + p->toString());
                auto curList = p->toVector();
                resultList.insert(resultList.end(), curList.begin(), curList.end());
            }
            return ListValue::fromVector(resultList);
        }

        ValuePtr car(const ValueList& params, EvalEnv& e)
        {
            if (!params[0]->isType(ValueType::PairType))
                throw LispError("Argument is not pair.");
            return static_pointer_cast<PairValue>(params[0])->left();
        }

        ValuePtr cdr(const ValueList& params, EvalEnv& e)
        {
            if (!params[0]->isType(ValueType::PairType))
                throw LispError("Argument is not pair.");
            return static_pointer_cast<PairValue>(params[0])->right();
        }

        ValuePtr cons(const ValueList& params, EvalEnv& e)
        {
            return make_shared<PairValue>(params[0], params[1]);
        }

        ValuePtr length(const ValueList& params, EvalEnv& e)
        {
            if (!params[0]->isType(ValueType::ListType))
                throw LispError("Malformed list: expected pair of nil, got " + params[0]->toString());
            return make_shared<NumericValue>(params[0]->toVector().size());
        }

        ValuePtr list(const ValueList& params, EvalEnv& e)
        {
            return ListValue::fromVector(params);
        }
    }

    namespace Math
    {
        ValuePtr add(const ValueList& params, EvalEnv& e)
        {
            double result = 0;
            for (const auto& i : params)
            {
                auto val = i->asNumber();
                if (!val)
                {
                    throw LispError("Cannot add a non-numeric value.");
                }
                result += *val;
            }
            return std::make_shared<NumericValue>(result);
        }

        ValuePtr minus(const ValueList& params, EvalEnv& e)
        {
            switch (params.size())
            {
            case 1:
            {
                return make_shared<NumericValue>(-*params[0]->asNumber());
            }
            case 2:
            {
                return make_shared<NumericValue>(*params[0]->asNumber() - *params[1]->asNumber());
            }
            default:
                break;
            }
        }

        ValuePtr multiply(const ValueList& params, EvalEnv& e)
        {
            double result = 1;
            for (auto& value : params)
            {
                result *= *value->asNumber();
            }
            return make_shared<NumericValue>(result);
        }

        ValuePtr divide(const ValueList& params, EvalEnv& e)
        {
            double x = 1, y = 0;
            switch (params.size())
            {
            case 1:
            {
                x = 1;
                y = *params[0]->asNumber();
                break;
            }
            case 2:
            {
                x = *params[0]->asNumber();
                y = *params[1]->asNumber();
                break;
            }
            default:
                break;
            }

            if (y == 0)
                throw LispError("Divided by 0");
            return make_shared<NumericValue>(x / y);
        }

        ValuePtr abs(const ValueList& params, EvalEnv& e)
        {
            return make_shared<NumericValue>(std::abs(*params[0]->asNumber()));
        }

        ValuePtr expt(const ValueList& params, EvalEnv& e)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            if (x == 0 && y == 0)
            {
                throw LispError("Not a number");
            }
            if (x > 0)
            {
                return make_shared<NumericValue>(std::pow(x, y));
            }
            else
            {
                auto cx = std::complex<double>(x, 0);
                auto result = std::pow(cx, y);
                if (result.imag() == 0)
                    return make_shared<NumericValue>(result.real());
                else
                    throw LispError("Not a number");
            }
        }

        ValuePtr quotient(const ValueList& params, EvalEnv& e)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            if (y == 0)
                throw LispError("Divided by 0");
            double result = x / y;
            return make_shared<NumericValue>(intNearZero(result));
        }

        ValuePtr remainder(const ValueList& params, EvalEnv& e)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            if (y == 0)
                throw LispError("Divided by 0");
            return make_shared<NumericValue>(x - y * intNearZero(x / y));
        }

        ValuePtr modulo(const ValueList& params, EvalEnv& e)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            double result = x;
            if (y != 0)
            {
                result = x - intNearZero(x / y) * y;
                if (result < 0 && y > 0)
                    result += y;
                if (result > 0 && y < 0)
                    result -= y;
            }
            return make_shared<NumericValue>(result);
        }

    }

    namespace Compare
    {
        ValuePtr equal(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(params[0]->getTypeID() == params[1]->getTypeID() && params[0]->toString() == params[1]->toString());
        }

        ValuePtr less(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() < *params[1]->asNumber());
        }

        ValuePtr more(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() > *params[1]->asNumber());
        }

        ValuePtr lessOrEqual(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() <= *params[1]->asNumber());
        }

        ValuePtr moreOrEqual(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() >= *params[1]->asNumber());
        }

        ValuePtr isEven(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(static_pointer_cast<NumericValue>(params[0])->isInteger() && (static_cast<long long>(*params[0]->asNumber()) % 2 == 0));
        }

        ValuePtr isOdd(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(static_pointer_cast<NumericValue>(params[0])->isInteger() && (static_cast<long long>(*params[0]->asNumber()) % 2 == 1));
        }

        ValuePtr isZero(const ValueList& params, EvalEnv& e)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() == 0);
        }

    }
}

using namespace Builtin::Helper;

unordered_map<string, ProcPtr> allBuiltins =
{
    BuiltinItem("print"s, Builtin::Core::print),
    BuiltinItem("display"s, Builtin::Core::display),
    BuiltinItem("exit"s, Builtin::Core::exit, ProcValue::UnlimitedCnt, 1),
    BuiltinItem("newline"s, Builtin::Core::newline),
    BuiltinItem("atom?"s, Builtin::TypeCheck::isType<ValueType::AtomType>, 1, 1),
    BuiltinItem("boolean?"s, Builtin::TypeCheck::isType<ValueType::BooleanType>, 1,1),
    BuiltinItem("number?"s, Builtin::TypeCheck::isType<ValueType::NumericType>, 1, 1),
    BuiltinItem("null?"s, Builtin::TypeCheck::isType<ValueType::NilType>, 1, 1),
    BuiltinItem("pair?"s, Builtin::TypeCheck::isType<ValueType::PairType>, 1, 1),
    BuiltinItem("procedure?"s, Builtin::TypeCheck::isType<ValueType::ProcedureType>, 1, 1),
    BuiltinItem("string?"s, Builtin::TypeCheck::isType<ValueType::StringType>, 1, 1),
    BuiltinItem("symbol?"s, Builtin::TypeCheck::isType<ValueType::SymbolType>, 1, 1),
    BuiltinItem("integer?"s, Builtin::TypeCheck::isInteger, 1, 1),
    BuiltinItem("list?"s, Builtin::TypeCheck::isList, 1, 1),
    BuiltinItem("append"s, Builtin::ListOperator::append),
    BuiltinItem("car"s, Builtin::ListOperator::car, 1),
    BuiltinItem("cdr"s, Builtin::ListOperator::cdr, 1),
    BuiltinItem("cons"s, Builtin::ListOperator::cons, 2),
    BuiltinItem("length"s, Builtin::ListOperator::length, 1),
    BuiltinItem("list"s, Builtin::ListOperator::list),
    BuiltinItem("+"s, Builtin::Math::add, ProcValue::UnlimitedCnt, ProcValue::UnlimitedCnt, {ValueType::NumericType, ProcValue::SameToRest}),
    BuiltinItem("-"s, Builtin::Math::minus, 1, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("*"s, Builtin::Math::multiply, ProcValue::UnlimitedCnt, ProcValue::UnlimitedCnt, {ValueType::NumericType, ProcValue::SameToRest}),
    BuiltinItem("/"s, Builtin::Math::divide, 1, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("abs"s, Builtin::Math::abs, 1, 1, {ValueType::NumericType}),
    BuiltinItem("expt"s, Builtin::Math::expt, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("quotient"s, Builtin::Math::quotient, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("remainder"s, Builtin::Math::remainder, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("modulo"s, Builtin::Math::modulo, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("="s, Builtin::Compare::equal, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("<"s, Builtin::Compare::less, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem(">"s, Builtin::Compare::more, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("<="s, Builtin::Compare::lessOrEqual, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem(">="s, Builtin::Compare::moreOrEqual, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("even?"s, Builtin::Compare::isEven, 1, 1, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("odd?"s, Builtin::Compare::isOdd, 1, 1, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("zero?"s, Builtin::Compare::isZero, 1, 1, {ValueType::NumericType, ValueType::NumericType}),
};

