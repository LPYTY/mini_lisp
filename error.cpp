#include "error.h"

int ExitEvent::exitCode() const
{
    return std::stoi(what());
}
