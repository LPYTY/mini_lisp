#ifndef BUILTINS_H
#define BUILTINS_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "./value.h"

using std::cout, std::vector, std::to_string, std::make_shared, std::unordered_map;

namespace Builtin
{
    namespace Helper // Not in builtin functions list
    {
    }
    using namespace Builtin::Helper;

    ValuePtr add(const vector<ValuePtr>& params);
    ValuePtr print(const vector<ValuePtr>& params);
}

extern unordered_map<string, ProcPtr> allBuiltins;

#endif // !BUILTINS_H

