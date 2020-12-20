#pragma once
#include <string>

class Sequence
{
public:
    Sequence(std::string InName, bool IsNested);
    const std::string Name;
    const bool bIsNested;

private:
    Sequence() = delete;
};
