#pragma once
#include "Sequence.h"
#include <vector>
#include <memory>
#include <string>

class IndividualSequence;

class NestedSequence : public Sequence
{
public:
    NestedSequence(std::string InName);

    std::shared_ptr<IndividualSequence> GetSubsequence(int Index);

    void AddSequence(std::shared_ptr<IndividualSequence> InSequence);
    void RemoveSequence(size_t Index);

private:
    std::vector<std::shared_ptr<IndividualSequence>> Sequences;

    NestedSequence() = delete;
};
