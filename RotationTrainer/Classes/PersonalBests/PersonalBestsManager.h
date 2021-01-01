#pragma once
#include "PersonalBestsClasses.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <string>
#include <vector>

class SequenceBestList;
class PersonalBest;

class PersonalBestsManager
{
public:
    PersonalBestsManager(std::shared_ptr<GameWrapper> InGameWrapper);

    void WriteRecord(const std::string& SequenceName, float CompletedTime, int SkippedSteps);
    std::vector<PersonalBest> GetBestTimesForSequence(const std::string& InSequenceName);

private:
    std::shared_ptr<GameWrapper> gameWrapper;

    std::vector<SequenceBestList> GetRecords();
    void AddNewPersonalBest(std::vector<SequenceBestList>& Records, const std::string& SequenceName, const PersonalBest& NewPersonalBest);
    void WriteToFile(const std::vector<SequenceBestList>& Records);

    PersonalBestsManager() = delete;
};
