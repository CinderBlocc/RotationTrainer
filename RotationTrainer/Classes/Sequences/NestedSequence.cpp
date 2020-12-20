#include "NestedSequence.h"
#include "IndividualSequence.h"

NestedSequence::NestedSequence(std::string InName) : Sequence(InName, true) {}

void NestedSequence::AddSequence(std::shared_ptr<IndividualSequence> InSequence)
{
    Sequences.push_back(InSequence);
}

void NestedSequence::RemoveSequence(size_t Index)
{
    if(Index >= Sequences.size())
    {
        return;
    }

    Sequences.erase(Sequences.begin() + Index);
}
