#include "./builtins.h"
#include "./eval_env.h"

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

        double numberConv(ValuePtr value)
        {
            return *value->asNumber();
        }

        string stringConv(ValuePtr value)
        {
            return std::dynamic_pointer_cast<StringValue>(value)->value();
        }

        string stringCiConv(ValuePtr value)
        {
            return ci(stringConv(value));
        }

        char charConv(ValuePtr value)
        {
            return std::dynamic_pointer_cast<CharValue>(value)->value();
        }

        char charCiConv(ValuePtr value)
        {
            return std::tolower(std::dynamic_pointer_cast<CharValue>(value)->value());
        }

        string ci(const string& s)
        {
            string result;
            std::ranges::transform(s, std::back_inserter(result), std::tolower);
            return result;
        }
    }

    namespace Core
    {
        ValuePtr apply(const ValueList& params, EvalEnv& env)
        {
            return env.apply(params[0], params[1]);
        }

        ValuePtr print(const ValueList& params, EvalEnv& env)
        {
            for (auto& p : params)
                cout << p->toString() << endl;
            return make_shared<NilValue>();
        }

        ValuePtr display(const ValueList& params, EvalEnv& env)
        {
            for (auto& p : params)
            {
                cout << p->toDisplayString() << endl;
            }
            return make_shared<NilValue>();
        }

        ValuePtr displayln(const ValueList& params, EvalEnv& env)
        {
            return display(params, env), newline(params, env);
        }

        ValuePtr error(const ValueList& params, EvalEnv& env)
        {
            throw LispError(params[0]->toString());
        }

        ValuePtr eval(const ValueList& params, EvalEnv& env)
        {
            return env.eval(params[0]);
        }

        ValuePtr exit(const ValueList& params, EvalEnv& env)
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

        ValuePtr newline(const ValueList& params, EvalEnv& env)
        {
            cout << endl;
            return make_shared<NilValue>();
        }

        ValuePtr read(const ValueList& params, EvalEnv& env)
        {
            return stdinReader->read();
        }
    }

    namespace TypeCheck
    {
        ValuePtr isInteger(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(params[0]->isType(ValueType::NumericType) && static_pointer_cast<NumericValue>(params[0])->isInteger());
        }

        ValuePtr isList(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(params[0]->isType(ValueType::ListType) && static_pointer_cast<ListValue>(params[0])->isList());
        }
    }

    namespace ListOperator
    {
        ValuePtr append(const ValueList& params, EvalEnv& env)
        {
            if (params.size() == 0)
                return make_shared<NilValue>();
            ValueList resultList;
            for (size_t i = 0; i < params.size(); i++)
            {
                auto curList = params[i]->toVector();
                if (i != params.size() - 1)
                    for (auto& p : curList)
                    {
                        p = p->copy();
                    }
                resultList.insert(resultList.end(), curList.begin(), curList.end());
            }
            return ListValue::fromVector(resultList);
        }

        ValuePtr car(const ValueList& params, EvalEnv& env)
        {
            if (!params[0]->isType(ValueType::PairType))
                throw LispError("Argument is not pair.");
            return static_pointer_cast<PairValue>(params[0])->left();
        }

        ValuePtr cdr(const ValueList& params, EvalEnv& env)
        {
            if (!params[0]->isType(ValueType::PairType))
                throw LispError("Argument is not pair.");
            return static_pointer_cast<PairValue>(params[0])->right();
        }

        ValuePtr cons(const ValueList& params, EvalEnv& env)
        {
            return make_shared<PairValue>(params[0]->copy(), params[1]->copy());
        }

        ValuePtr length(const ValueList& params, EvalEnv& env)
        {
            if (!params[0]->isType(ValueType::ListType))
                throw LispError("Malformed list: expected pair of nil, got " + params[0]->toString());
            return make_shared<NumericValue>(params[0]->toVector().size());
        }

        ValuePtr list(const ValueList& params, EvalEnv& env)
        {
            ValueList v;
            std::ranges::transform(params, std::back_inserter(v), [](auto& p) {return p->copy(); });
            return ListValue::fromVector(v);
        }

        ValuePtr map(const ValueList& params, EvalEnv& env)
        {
            auto proc = static_pointer_cast<CallableValue>(params[0]);
            ValueList resultList;
            vector<ValueList> paramLists;
            paramLists.push_back(params[1]->toVector());
            size_t paramListSize = paramLists[0].size();
            for (size_t i = 2; i < params.size(); i++)
            {
                auto nextList = params[i]->toVector();
                if (nextList.size() != paramListSize)
                    throw LispError("Param lists mismatch.");
                paramLists.push_back(nextList);
            }
            size_t maxSize = 0;
            for (auto& list : paramLists)
            {
                if (list.size() > maxSize)
                    maxSize = list.size();
            }
            for (size_t i = 0; i < paramListSize; i++)
            {
                ValueList args;
                for (auto& paramList : paramLists)
                {
                    args.push_back(paramList[i]);
                }
                resultList.push_back(env.apply(proc, args));
            }
            return ListValue::fromVector(resultList);
        }

        ValuePtr filter(const ValueList& params, EvalEnv& env)
        {
            auto proc = static_pointer_cast<ProcValue>(params[0]);
            auto paramList = params[1]->toVector();
            ValueList resultList;
            for (auto& value : paramList)
            {
                if (*env.apply(proc, ValueList{ value }))
                    resultList.push_back(value);
            }
            return ListValue::fromVector(resultList);
        }

        ValuePtr reduce(const ValueList& params, EvalEnv& env)
        {
            auto proc = static_pointer_cast<CallableValue>(params[0]);
            auto paramList = params[1]->toVector();
            switch (paramList.size())
            {
            case 0:
                throw LispError("reduce list must have at least 1 element");
            case 1:
                return paramList[0];
            default:
            {
                auto paramPair = static_pointer_cast<PairValue>(params[1]);
                return env.apply(proc, { paramPair->left(), reduce({proc, paramPair->right()},env) });
            }
            }
        }

    }

    namespace Math
    {
        ValuePtr add(const ValueList& params, EvalEnv& env)
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

        ValuePtr minus(const ValueList& params, EvalEnv& env)
        {
            switch (params.size())
            {
            case 1:
            {
                return make_shared<NumericValue>(-*params[0]->asNumber());
            }
            default:
                return make_shared<NumericValue>(*params[0]->asNumber() - *params[1]->asNumber());
            }
        }

        ValuePtr multiply(const ValueList& params, EvalEnv& env)
        {
            double result = 1;
            for (auto& value : params)
            {
                result *= *value->asNumber();
            }
            return make_shared<NumericValue>(result);
        }

        ValuePtr divide(const ValueList& params, EvalEnv& env)
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

        ValuePtr abs(const ValueList& params, EvalEnv& env)
        {
            return make_shared<NumericValue>(std::abs(*params[0]->asNumber()));
        }

        ValuePtr expt(const ValueList& params, EvalEnv& env)
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

        ValuePtr quotient(const ValueList& params, EvalEnv& env)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            if (y == 0)
                throw LispError("Divided by 0");
            double result = x / y;
            return make_shared<NumericValue>(static_cast<long long>(result));
        }

        ValuePtr remainder(const ValueList& params, EvalEnv& env)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            if (y == 0)
                throw LispError("Divided by 0");
            return make_shared<NumericValue>(x - y * static_cast<long long>(x / y));
        }

        ValuePtr modulo(const ValueList& params, EvalEnv& env)
        {
            double x = *params[0]->asNumber(), y = *params[1]->asNumber();
            double result = x;
            if (y != 0)
            {
                result = x - static_cast<long long>(x / y) * y;
                if (result * y < 0)
                    result += y;
                //if (result > 0 && y < 0)
                    //result -= y;
            }
            return make_shared<NumericValue>(result);
        }

        ValuePtr gcd(const ValueList& params, EvalEnv& e)
        {
            if (!std::dynamic_pointer_cast<NumericValue>(params[0])->isInteger() && std::dynamic_pointer_cast<NumericValue>(params[1])->isInteger())
            {
                throw LispError("gcd only works on two integers");
            }
            long long x = std::abs(*params[0]->asNumber()), y = std::abs(*params[1]->asNumber());
            if (x == 0 || y == 0)
                return make_shared<NumericValue>(0);
            else
            {
                while (x != 0 && y != 0)
                    if (x > y)
                        x = x % y;
                    else
                        y = y % x;
            }
            return make_shared<NumericValue>(x + y);
        }

        ValuePtr lcm(const ValueList& params, EvalEnv& e)
        {
            if (!std::dynamic_pointer_cast<NumericValue>(params[0])->isInteger() && std::dynamic_pointer_cast<NumericValue>(params[1])->isInteger())
            {
                throw LispError("lcm only works on two integers");
            }
            long long x = std::abs(*params[0]->asNumber()), y = std::abs(*params[1]->asNumber());
            return make_shared<NumericValue>(x * y / (*gcd(params, e)->asNumber()));
        }

    }

    namespace String
    {
        ValuePtr makeString(const ValueList& params, EvalEnv& env)
        {
            size_t n = *params[0]->asNumber();
            char filler = ' ';
            if (params.size() >= 2)
                filler = std::dynamic_pointer_cast<CharValue>(params[1])->value();
            return make_shared<StringValue>(string(n, filler));
        }

        ValuePtr _string(const ValueList& params, EvalEnv& env)
        {
            string result;
            for (auto p : params)
            {
                result += std::dynamic_pointer_cast<CharValue>(p)->value();
            }
            return make_shared<StringValue>(result);
        }

        ValuePtr stringLength(const ValueList& params, EvalEnv& env)
        {
            return make_shared<NumericValue>(std::dynamic_pointer_cast<StringValue>(params[0])->value().size());
        }

        ValuePtr stringRef(const ValueList& params, EvalEnv& env)
        {
            auto str = std::dynamic_pointer_cast<StringValue>(params[0]);
            if (!*TypeCheck::isInteger({ params[1] }, env))
                throw LispError("Index is required to be an integer");
            long long index = static_cast<long long>(*params[1]->asNumber());
            return make_shared<CharValue>(str->at(index));
        }

        ValuePtr stringSet(const ValueList& params, EvalEnv& env)
        {
            auto str = std::dynamic_pointer_cast<StringValue>(params[0]);
            if (!*TypeCheck::isInteger({ params[1] }, env))
                throw LispError("Index is required to be an integer");
            long long index = static_cast<long long>(*params[1]->asNumber());
            char newChar = std::dynamic_pointer_cast<CharValue>(params[2])->value();
            str->at(index) = newChar;
            return make_shared<NilValue>();
        }

        ValuePtr subString(const ValueList& params, EvalEnv& env)
        {
            const string& originalString = std::dynamic_pointer_cast<StringValue>(params[0])->value();
            if (!std::dynamic_pointer_cast<NumericValue>(params[1])->isInteger() || !std::dynamic_pointer_cast<NumericValue>(params[2])->isInteger())
                throw LispError("Index must be integer");
            long long start = *params[1]->asNumber();
            long long end = *params[2]->asNumber();
            if (start < 0)
                throw LispError("Start position should not be negative");
            if (end < start)
                throw LispError("End position should not be smaller than start position");
            if (end > originalString.size())
                throw LispError("Index out of range");
            return make_shared<StringValue>(originalString.substr(start, end - start));
        }

        ValuePtr stringAppend(const ValueList& params, EvalEnv& env)
        {
            string result;
            for (auto& param : params)
            {
                result += stringConv(param);
            }
            return make_shared<StringValue>(result);
        }

        ValuePtr listToString(const ValueList& params, EvalEnv& env)
        {
            string result;
            ValueList chars = params[0]->toVector();
            for (auto& character : chars)
            {
                if (!character->isType(ValueType::CharType))
                    throw LispError("A list of characters expected");
                result += std::dynamic_pointer_cast<CharValue>(character)->value();
            }
            return make_shared<StringValue>(result);
        }

        ValuePtr stringToList(const ValueList& params, EvalEnv& env)
        {
            vector<ValuePtr> result;
            string& str = std::dynamic_pointer_cast<StringValue>(params[0])->value();
            for (auto& character : str)
            {
                result.push_back(make_shared<CharValue>(character));
            }
            return ListValue::fromVector(result);
        }

        ValuePtr stringCopy(const ValueList& params, EvalEnv& env)
        {
            return make_shared<StringValue>(std::dynamic_pointer_cast<StringValue>(params[0])->value());
        }

        ValuePtr stringFill(const ValueList& params, EvalEnv& env)
        {
            string& str = std::dynamic_pointer_cast<StringValue>(params[0])->value();
            char filler = std::dynamic_pointer_cast<CharValue>(params[1])->value();
            for (auto& character : str)
            {
                character = filler;
            }
            return make_shared<NilValue>();
        }

        BuiltinFunc stringEqual = std::bind(compare<string>(), _1, isEqual<string>(), stringConv);
        BuiltinFunc stringEqualCi = std::bind(compare<string>(), _1, isEqual<string>(), stringCiConv);
        BuiltinFunc stringGreater = std::bind(compare<string>(), _1, std::greater<string>(), stringConv);
        BuiltinFunc stringSmaller = std::bind(compare<string>(), _1, std::less<string>(), stringConv);
        BuiltinFunc stringGreaterOrEqual = std::bind(compare<string>(), _1, std::greater_equal<string>(), stringConv);
        BuiltinFunc stringSmallerOrEqual = std::bind(compare<string>(), _1, std::less_equal<string>(), stringConv);
        BuiltinFunc stringGreaterCi = std::bind(compare<string>(), _1, std::greater<string>(), stringCiConv);
        BuiltinFunc stringSmallerCi = std::bind(compare<string>(), _1, std::less<string>(), stringCiConv);
        BuiltinFunc stringGreaterOrEqualCi = std::bind(compare<string>(), _1, std::greater_equal<string>(), stringCiConv);
        BuiltinFunc stringSmallerOrEqualCi = std::bind(compare<string>(), _1, std::less_equal<string>(), stringCiConv);
    }

    namespace Char
    {
        BuiltinFunc charEqual = std::bind(compare<char>(), _1, isEqual<char>(), charConv);
        BuiltinFunc charEqualCi = std::bind(compare<char>(), _1, isEqual<char>(), charCiConv);
        BuiltinFunc charGreater = std::bind(compare<char>(), _1, std::greater<char>(), charConv);
        BuiltinFunc charSmaller = std::bind(compare<char>(), _1, std::less<char>(), charConv);
        BuiltinFunc charGreaterOrEqual = std::bind(compare<char>(), _1, std::greater_equal<char>(), charConv);
        BuiltinFunc charSmallerOrEqual = std::bind(compare<char>(), _1, std::less_equal<char>(), charConv);
        BuiltinFunc charGreaterCi = std::bind(compare<char>(), _1, std::greater<char>(), charCiConv);
        BuiltinFunc charSmallerCi = std::bind(compare<char>(), _1, std::less<char>(), charCiConv);
        BuiltinFunc charGreaterOrEqualCi = std::bind(compare<char>(), _1, std::greater_equal<char>(), charCiConv);
        BuiltinFunc charSmallerOrEqualCi = std::bind(compare<char>(), _1, std::less_equal<char>(), charCiConv);

        ValuePtr isCharAlphabetic(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<BooleanValue>(std::isalpha(c));
        }

        ValuePtr isCharNumeric(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<BooleanValue>(std::isdigit(c));
        }

        ValuePtr isCharWhitespace(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<BooleanValue>(std::isspace(c));
        }

        ValuePtr isCharUpperCase(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<BooleanValue>(std::isupper(c));
        }

        ValuePtr isCharLowerCase(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<BooleanValue>(std::islower(c));
        }

        ValuePtr charToInteger(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<NumericValue>(static_cast<long long>(c));
        }

        ValuePtr integerToChar(const ValueList& params, EvalEnv& env)
        {
            long long n = *params[0]->asNumber();
            return make_shared<CharValue>(static_cast<char>(n));
        }

        ValuePtr charUpcase(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<CharValue>(std::toupper(c));
        }

        ValuePtr charDowncase(const ValueList& params, EvalEnv& env)
        {
            char c = std::dynamic_pointer_cast<CharValue>(params[0])->value();
            return make_shared<CharValue>(std::tolower(c));
        }
    }

    namespace Vector
    {
        ValuePtr vectorFill(const ValueList& params, EvalEnv& env)
        {
            auto& v = std::dynamic_pointer_cast<VectorValue>(params[0])->value();
            ValuePtr filler = params[1];
            for (auto& p : v)
            {
                p = filler->copy();
            }
            return make_shared<NilValue>();
        }

        ValuePtr makeVector(const ValueList& params, EvalEnv& env)
        {
            auto n = std::dynamic_pointer_cast<NumericValue>(params[0]);
            if (!n->isInteger())
                throw LispError("k should be an integer");
            long long k = *n->asNumber();
            if (k < 0)
                throw LispError("k should be non-negative");
            ValuePtr filler;
            if (params.size() >= 2)
                filler = params[1];
            else
                filler = make_shared<NilValue>();
            ValueList result;
            for (long long i = 0; i < k; i++)
                result.push_back(filler->copy());
            return make_shared<VectorValue>(result);
        }

        ValuePtr _vector(const ValueList& params, EvalEnv& env)
        {
            return make_shared<VectorValue>(params);
        }

        ValuePtr vectorRef(const ValueList& params, EvalEnv& env)
        {
            auto v = std::dynamic_pointer_cast<VectorValue>(params[0]);
            auto n = std::dynamic_pointer_cast<NumericValue>(params[1]);
            if (!n->isInteger())
                throw LispError("Index should be an integer");
            long long index = *n->asNumber();
            return v->at(index);
        }

        ValuePtr vectorLength(const ValueList& params, EvalEnv& env)
        {
            return make_shared<NumericValue>(std::dynamic_pointer_cast<VectorValue>(params[0])->value().size());
        }

        ValuePtr vectorSet(const ValueList& params, EvalEnv& env)
        {
            auto v = std::dynamic_pointer_cast<VectorValue>(params[0]);
            auto n = std::dynamic_pointer_cast<NumericValue>(params[1]);
            if (!n->isInteger())
                throw LispError("Index should be an integer");
            long long index = *n->asNumber();
            auto& p = v->at(index);
            p = params[2];
            return make_shared<NilValue>();
        }

        ValuePtr vectorToList(const ValueList& params, EvalEnv& env)
        {
            auto v = std::dynamic_pointer_cast<VectorValue>(params[0])->value();
            for (auto& p : v)
            {
                p = p->copy();
            }
            return ListValue::fromVector(v);
        }

        ValuePtr listToVector(const ValueList& params, EvalEnv& env)
        {
            auto v = params[0]->toVector();
            for (auto& p : v)
            {
                p = p->copy();
            }
            return make_shared<VectorValue>(v);
        }
    }

    namespace Compare
    {
        ValuePtr eq(const ValueList& params, EvalEnv& env)
        {
            if (params[0]->getTypeID() != params[1]->getTypeID())
                return make_shared<BooleanValue>(false);
            if (params[0]->isType(
                ValueType::BooleanType |
                ValueType::NumericType |
                ValueType::CallableType |
                ValueType::NilType |
                ValueType::SymbolType |
                ValueType::CharType
            ))
                return make_shared<BooleanValue>(params[0]->toString() == params[1]->toString());
            else
                return make_shared<BooleanValue>(params[0] == params[1]);
        }

        ValuePtr equal(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(params[0]->getTypeID() == params[1]->getTypeID() && params[0]->toString() == params[1]->toString());
        }

        ValuePtr _not(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(!*params[0]);
        }

        BuiltinFunc less = std::bind(compare<double>(), _1, std::less<double>(), numberConv);
        BuiltinFunc more = std::bind(compare<double>(), _1, std::greater<double>(), numberConv);
        BuiltinFunc lessOrEqual = std::bind(compare<double>(), _1, std::less_equal<double>(), numberConv);
        BuiltinFunc moreOrEqual = std::bind(compare<double>(), _1, std::greater_equal<double>(), numberConv);

        ValuePtr isEven(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(static_pointer_cast<NumericValue>(params[0])->isInteger() && (std::abs(static_cast<long long>(*params[0]->asNumber())) % 2 == 0));
        }

        ValuePtr isOdd(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(static_pointer_cast<NumericValue>(params[0])->isInteger() && (std::abs(static_cast<long long>(*params[0]->asNumber())) % 2 == 1));
        }

        ValuePtr isZero(const ValueList& params, EvalEnv& env)
        {
            return make_shared<BooleanValue>(*params[0]->asNumber() == 0);
        }

    }

    namespace Control
    {
        ValuePtr force(const ValueList& params, EvalEnv& env)
        {
            return static_pointer_cast<PromiseValue>(params[0])->force(env);
        }
    }
}

using namespace Builtin::Helper;

unordered_map<string, CallablePtr> allBuiltins =
{
    BuiltinItem("apply"s, Builtin::Core::apply, 2, 2, {ValueType::ProcedureType, ValueType::ListType}),
    BuiltinItem("print"s, Builtin::Core::print),
    BuiltinItem("display"s, Builtin::Core::display),
    BuiltinItem("displayln"s, Builtin::Core::displayln),
    BuiltinItem("error"s, Builtin::Core::error, 1),
    BuiltinItem("eval"s, Builtin::Core::eval, 1, 1),
    BuiltinItem("exit"s, Builtin::Core::exit, CallableValue::UnlimitedCnt, 1),
    BuiltinItem("newline"s, Builtin::Core::newline),
    BuiltinItem("read"s, Builtin::Core::read, 0, 0),

    BuiltinItem("atom?"s, Builtin::TypeCheck::isType<ValueType::AtomType>, 1, 1),
    BuiltinItem("boolean?"s, Builtin::TypeCheck::isType<ValueType::BooleanType>, 1,1),
    BuiltinItem("number?"s, Builtin::TypeCheck::isType<ValueType::NumericType>, 1, 1),
    BuiltinItem("null?"s, Builtin::TypeCheck::isType<ValueType::NilType>, 1, 1),
    BuiltinItem("pair?"s, Builtin::TypeCheck::isType<ValueType::PairType>, 1, 1),
    BuiltinItem("procedure?"s, Builtin::TypeCheck::isType<ValueType::ProcedureType>, 1, 1),
    BuiltinItem("string?"s, Builtin::TypeCheck::isType<ValueType::StringType>, 1, 1),
    BuiltinItem("symbol?"s, Builtin::TypeCheck::isType<ValueType::SymbolType>, 1, 1),
    BuiltinItem("char?"s, Builtin::TypeCheck::isType<ValueType::CharType>, 1, 1),
    BuiltinItem("vector?"s, Builtin::TypeCheck::isType<ValueType::VectorType>, 1, 1),
    BuiltinItem("integer?"s, Builtin::TypeCheck::isInteger, 1, 1),
    BuiltinItem("list?"s, Builtin::TypeCheck::isList, 1, 1),

    BuiltinItem("append"s, Builtin::ListOperator::append),
    BuiltinItem("car"s, Builtin::ListOperator::car, 1),
    BuiltinItem("cdr"s, Builtin::ListOperator::cdr, 1),
    BuiltinItem("cons"s, Builtin::ListOperator::cons, 2),
    BuiltinItem("length"s, Builtin::ListOperator::length, 1),
    BuiltinItem("list"s, Builtin::ListOperator::list),
    BuiltinItem("map"s, Builtin::ListOperator::map, 2, CallableValue::UnlimitedCnt, {ValueType::ProcedureType, ValueType::ListType, CallableValue::SameToRest}),
    BuiltinItem("filter"s, Builtin::ListOperator::filter, 2, 2, {ValueType::ProcedureType, ValueType::ListType}),
    BuiltinItem("reduce"s, Builtin::ListOperator::reduce, 2, 2,{ValueType::ProcedureType, ValueType::ListType}),

    BuiltinItem("+"s, Builtin::Math::add, CallableValue::UnlimitedCnt, CallableValue::UnlimitedCnt, {ValueType::NumericType, CallableValue::SameToRest}),
    BuiltinItem("-"s, Builtin::Math::minus, 1, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("*"s, Builtin::Math::multiply, CallableValue::UnlimitedCnt, CallableValue::UnlimitedCnt, {ValueType::NumericType, CallableValue::SameToRest}),
    BuiltinItem("/"s, Builtin::Math::divide, 1, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("abs"s, Builtin::Math::abs, 1, 1, {ValueType::NumericType}),
    BuiltinItem("expt"s, Builtin::Math::expt, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("quotient"s, Builtin::Math::quotient, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("remainder"s, Builtin::Math::remainder, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("modulo"s, Builtin::Math::modulo, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("gcd"s,Builtin::Math::gcd,2,2,{ValueType::NumericType,ValueType::NumericType}),
    BuiltinItem("lcm"s,Builtin::Math::lcm,2,2,{ValueType::NumericType,ValueType::NumericType}),

    BuiltinItem("eq?"s, Builtin::Compare::eq, 2, 2),
    BuiltinItem("equal?"s, Builtin::Compare::equal, 2, 2),
    BuiltinItem("not"s, Builtin::Compare::_not, 1),
    BuiltinItem("="s, Builtin::Compare::equal, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("<"s, Builtin::Compare::less, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem(">"s, Builtin::Compare::more, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("<="s, Builtin::Compare::lessOrEqual, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem(">="s, Builtin::Compare::moreOrEqual, 2, 2, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("even?"s, Builtin::Compare::isEven, 1, 1, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("odd?"s, Builtin::Compare::isOdd, 1, 1, {ValueType::NumericType, ValueType::NumericType}),
    BuiltinItem("zero?"s, Builtin::Compare::isZero, 1, 1, {ValueType::NumericType, ValueType::NumericType}),

    BuiltinItem("char=?"s,Builtin::Char::charEqual,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-ci=?"s,Builtin::Char::charEqualCi,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char>?"s,Builtin::Char::charGreater,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char<?"s,Builtin::Char::charSmaller,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char>=?"s,Builtin::Char::charGreaterOrEqual,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char<=?"s,Builtin::Char::charSmallerOrEqual,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-ci>?"s,Builtin::Char::charGreaterCi,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-ci<?"s,Builtin::Char::charSmallerCi,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-ci>=?"s,Builtin::Char::charGreaterOrEqualCi,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-ci<=?"s,Builtin::Char::charSmallerOrEqualCi,2,2,{ValueType::CharType,ValueType::CharType}),
    BuiltinItem("char-alphabetic?"s, Builtin::Char::isCharAlphabetic, 1, 1, {ValueType::CharType}),
    BuiltinItem("char-numeric?"s, Builtin::Char::isCharNumeric, 1, 1, {ValueType::CharType}),
    BuiltinItem("char-whitespace?"s, Builtin::Char::isCharWhitespace, 1, 1, {ValueType::CharType}),
    BuiltinItem("char-uppercase?"s, Builtin::Char::isCharUpperCase, 1, 1, {ValueType::CharType}),
    BuiltinItem("char-lowercase?"s, Builtin::Char::isCharLowerCase, 1, 1, {ValueType::CharType}),
    BuiltinItem("char->integer"s, Builtin::Char::charToInteger, 1, 1, {ValueType::CharType}),
    BuiltinItem("integer->char"s, Builtin::Char::integerToChar, 1, 1, {ValueType::NumericType}),
    BuiltinItem("char-upcase"s, Builtin::Char::charUpcase, 1, 1,{ValueType::CharType}),
    BuiltinItem("char-downcase"s, Builtin::Char::charDowncase, 1, 1,{ValueType::CharType}),



    BuiltinItem("make-string"s, Builtin::String::makeString, 1,2,{ValueType::NumericType,ValueType::CharType}),
    BuiltinItem("string"s,Builtin::String::_string,CallableValue::UnlimitedCnt,CallableValue::UnlimitedCnt,{ValueType::CharType,CallableValue::SameToRest}),
    BuiltinItem("string-length"s,Builtin::String::stringLength,1,1,{ValueType::StringType}),
    BuiltinItem("string-ref"s,Builtin::String::stringRef,2,2,{ValueType::StringType,ValueType::NumericType}),
    BuiltinItem("string-set!"s,Builtin::String::stringSet,3,3,{ValueType::StringType,ValueType::NumericType,ValueType::CharType}),
    BuiltinItem("string=?"s,Builtin::String::stringEqual,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string-ci=?"s,Builtin::String::stringEqualCi,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string>?"s,Builtin::String::stringGreater,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string<?"s,Builtin::String::stringSmaller,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string>=?"s,Builtin::String::stringGreaterOrEqual,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string<=?"s,Builtin::String::stringSmallerOrEqual,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string-ci>?"s,Builtin::String::stringGreaterCi,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string-ci<?"s,Builtin::String::stringSmallerCi,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string-ci>=?"s,Builtin::String::stringGreaterOrEqualCi,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("string-ci<=?"s,Builtin::String::stringSmallerOrEqualCi,2,2,{ValueType::StringType,ValueType::StringType}),
    BuiltinItem("substring"s,Builtin::String::subString,3,3,{ValueType::StringType,ValueType::NumericType,ValueType::NumericType}),
    BuiltinItem("string-append"s,Builtin::String::stringAppend,2,CallableValue::UnlimitedCnt,{ValueType::StringType,CallableValue::SameToRest}),
    BuiltinItem("string->list"s,Builtin::String::stringToList,1,1,{ValueType::StringType}),
    BuiltinItem("list->string"s,Builtin::String::listToString,1,1,{ValueType::ListType}),
    BuiltinItem("string-copy"s,Builtin::String::stringCopy,1,1,{ValueType::StringType}),
    BuiltinItem("string-fill!"s,Builtin::String::stringFill,2,2,{ValueType::StringType,ValueType::CharType}),

    BuiltinItem("make-vector"s, Builtin::Vector::makeVector, 1, 2, {ValueType::NumericType, ValueType::AllType}),
    BuiltinItem("vector"s, Builtin::Vector::_vector),
    BuiltinItem("vector-length"s, Builtin::Vector::vectorLength, 1, 1, { ValueType::VectorType }),
    BuiltinItem("vector-ref"s, Builtin::Vector::vectorRef, 2, 2, { ValueType::VectorType,ValueType::NumericType }),
    BuiltinItem("vector-set!"s, Builtin::Vector::vectorSet, 3, 3, { ValueType::VectorType,ValueType::NumericType,ValueType::AllType }),
    BuiltinItem("vector->list"s, Builtin::Vector::vectorToList, 1, 1, { ValueType::VectorType }),
    BuiltinItem("list->vector"s, Builtin::Vector::listToVector, 1, 1, { ValueType::ListType }),
    BuiltinItem("vector-fill!"s, Builtin::Vector::vectorFill, 2, 2, { ValueType::VectorType,ValueType::AllType }),

    BuiltinItem("force"s, Builtin::Control::force,1,1,{ValueType::PromiseType}),
};

