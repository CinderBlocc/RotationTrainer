#pragma once
#include "Sequence.h"
#include "SequenceProperties.h"
#include <vector>
#include <memory>
#include <string>

class Checkpoint;

class IndividualSequence : public Sequence
{
public:
    IndividualSequence(std::string InName, SequenceProperties InProperties);

    void AddCheckpoint(std::shared_ptr<Checkpoint> InCheckpoint);
    void RemoveCheckpoint(size_t Index);

private:
    std::vector<std::shared_ptr<Checkpoint>> Checkpoints;
    SequenceProperties Properties;

    IndividualSequence() = delete;
};
