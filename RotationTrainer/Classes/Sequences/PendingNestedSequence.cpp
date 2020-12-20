#include "PendingNestedSequence.h"
#include "IndividualSequence.h"
#include "NestedSequence.h"
#include "Sequence.h"

PendingNestedSequence::PendingNestedSequence(std::string InName) : Name(InName) {}

void PendingNestedSequence::AddSequenceRequest(std::string InSequenceName)
{
    RequestedSequenceNames.push_back(InSequenceName);
}

void PendingNestedSequence::RemoveSequenceRequest(size_t Index)
{
    if(Index >= RequestedSequenceNames.size())
    {
        return;
    }

    RequestedSequenceNames.erase(RequestedSequenceNames.begin() + Index);
}

std::shared_ptr<NestedSequence> PendingNestedSequence::CreateNestedSequence(const std::vector<std::shared_ptr<Sequence>>& AllSequences)
{
    //Converts the container of requested sequences into a full NestedSequence
    std::shared_ptr<NestedSequence> OutputSequence = std::make_shared<NestedSequence>(Name);

    //Loop through all requested sequence names and search for a corresponding IndividualSequence
    for(const auto& RequestedSequence : RequestedSequenceNames)
    {
        if(auto ThisSequence = FindIndividualSequence(RequestedSequence, AllSequences))
        {
            OutputSequence->AddSequence(ThisSequence);
        }
    }

    return OutputSequence;
}

std::shared_ptr<IndividualSequence> PendingNestedSequence::FindIndividualSequence(std::string InSequenceName, const std::vector<std::shared_ptr<Sequence>>& AllSequences)
{
    //AllSequences passed in from SequenceManager so this function can find the right InidividualSequence pointer
    for(const auto& ThisSequence : AllSequences)
    {
        if(!ThisSequence->bIsNested && ThisSequence->Name == InSequenceName)
        {
            //Cast from "Sequence" to "IndividualSequence"
            return std::static_pointer_cast<IndividualSequence>(ThisSequence);
        }
    }

    return nullptr;
}
