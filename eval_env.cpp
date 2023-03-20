#include "./eval_env.h"

ValuePtr EvalEnv::eval(ValuePtr expr)
{
    if (expr->isSelfEvaluating())
    {
        return expr;
    }
    if (expr->isNil())
    {
        throw LispError("Evaluating nil is prohibited.");
    }
    else
    {
        throw LispError("Unimplemented.");
    }
}
