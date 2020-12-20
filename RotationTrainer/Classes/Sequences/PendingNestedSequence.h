#pragma once
#include <string>
#include <vector>
#include <memory>

class IndividualSequence;
class NestedSequence;
class Sequence;

class PendingNestedSequence
{
public:
    PendingNestedSequence(std::string InName);

    void AddSequenceRequest(std::string InSequenceName);
    void RemoveSequenceRequest(size_t Index);

    std::shared_ptr<NestedSequence> CreateNestedSequence(const std::vector<std::shared_ptr<Sequence>>& AllSequences);

private:
    PendingNestedSequence() = delete;

    std::shared_ptr<IndividualSequence> FindIndividualSequence(std::string InSequenceName, const std::vector<std::shared_ptr<Sequence>>& AllSequences);

    std::string Name;
    std::vector<std::string> RequestedSequenceNames;
};
