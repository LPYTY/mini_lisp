#include "./builtins.h"

using namespace std::literals;
using std::make_pair;

namespace Builtin
{
    namespace Helper
    {
    }

    ValuePtr add(const std::vector<ValuePtr>& params)
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

    ValuePtr print(const vector<ValuePtr>& params)
    {
        for (auto& p : params)
            cout << p->toString() << endl;
        return make_shared<NilValue>();
    }

}


unordered_map<string, ProcPtr> allBuiltins =
{
    make_pair("+"s, make_shared<BuiltinProcValue>(Builtin::add)),
    make_pair("print"s, make_shared<BuiltinProcValue>(Builtin::print)),
};

