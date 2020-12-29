#pragma once
#include <vector>
#include <memory>
#include <string>
#include <fstream>

struct SequenceProperties;
class PendingNestedSequence;
class IndividualSequence;
class NestedSequence;
class Sequence;
class Checkpoint;

class SequenceContainer
{
public:
    SequenceContainer();

    std::shared_ptr<Sequence> GetSequence(const std::string& InSequenceName);

    void LoadAllSequences(std::filesystem::path InDirectory);
    const std::vector<std::string>& GetSequenceFilenames();

private:
    std::vector<std::string> AllSequenceFileNames;
    std::vector<std::shared_ptr<Sequence>> AllSequences;
    std::vector<std::shared_ptr<PendingNestedSequence>> AllPendingNestedSequences;
    std::vector<std::shared_ptr<Checkpoint>> DefaultCheckpoints;

    //Default checkpoints
    void FillDefaultCheckpoints();
    std::shared_ptr<Checkpoint> FindDefaultCheckpoint(const std::string& CheckpointName);

    //All sequences in folder
    void GetAllSequenceFiles(std::filesystem::path InDirectory);
    
    //Individual sequences
    SequenceProperties GetSequenceProperties(std::ifstream& InFile, bool& OutbIsNestedSequence);
    void AddIndividualSequence(std::ifstream& InFile, const std::string& Filename, SequenceProperties Properties);
    
    //Nested sequences
    void AddPendingNestedSequence(std::ifstream& InFile, const std::string& Filename);
    void CompletePendingNestedSequenceRequests();
};
