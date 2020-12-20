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

class SequenceManager
{
public:
    SequenceManager(std::vector<std::shared_ptr<Checkpoint>>* InCheckpoints);

    void LoadAllSequences(std::string InDirectory);
    const std::vector<std::string>& GetSequenceFilenames();

private:
    SequenceManager() = delete;

    std::vector<std::string> AllSequenceFileNames;
    std::vector<std::shared_ptr<Sequence>> AllSequences;
    std::vector<std::shared_ptr<PendingNestedSequence>> AllPendingNestedSequences;

    std::vector<std::shared_ptr<Checkpoint>>* AllCheckpoints;

    void GetAllSequenceFiles(std::string InDirectory);
    SequenceProperties GetSequenceProperties(std::ifstream& InFile, bool& OutbIsNestedSequence);
    void AddIndividualSequence(std::ifstream& InFile, const std::string& Filename, SequenceProperties Properties);
    std::shared_ptr<Checkpoint> FindCheckpoint(std::string CheckpointName);
    void AddPendingNestedSequence(std::ifstream& InFile, const std::string& Filename);
    void CompletePendingNestedSequenceRequests();
};
