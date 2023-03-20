#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include "./value.h"
#include "./error.h"

class EvalEnv
{
public:
    ValuePtr eval(ValuePtr expr);
};

#endif
