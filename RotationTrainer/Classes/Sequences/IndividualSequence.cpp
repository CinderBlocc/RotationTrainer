#include "IndividualSequence.h"
#include "Checkpoints/Checkpoint.h"

IndividualSequence::IndividualSequence(std::string InName, SequenceProperties InProperties) : Sequence(InName, false), Properties(InProperties) {}

void IndividualSequence::AddCheckpoint(std::shared_ptr<Checkpoint> InCheckpoint)
{
    Checkpoints.push_back(InCheckpoint);
}

void IndividualSequence::RemoveCheckpoint(size_t Index)
{
    if(Index >= Checkpoints.size())
    {
        return;
    }

    Checkpoints.erase(Checkpoints.begin() + Index);
}

void IndividualSequence::ResetAllCheckpoints()
{
    for(const auto& ThisCheckpoint : Checkpoints)
    {
        ThisCheckpoint->ResetCheckpoint();
    }
}

const std::shared_ptr<Checkpoint> IndividualSequence::GetCheckpoint(int Index)
{
    if(Index >= Checkpoints.size())
    {
        return nullptr;
    }

    return Checkpoints[Index];
}
