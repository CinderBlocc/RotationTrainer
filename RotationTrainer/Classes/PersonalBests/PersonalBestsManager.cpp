#include "PersonalBestsManager.h"
#include "MacrosStructsEnums.h"
#include <filesystem>
#include <fstream>


PersonalBestsManager::PersonalBestsManager(std::shared_ptr<GameWrapper> InGameWrapper)
    : gameWrapper(InGameWrapper) {}

void PersonalBestsManager::WriteRecord(const std::string& SequenceName, float CompletedTime, int SkippedSteps)
{
    //Check if the directory for the file exists
    if(!std::filesystem::exists(DEFAULT_BESTS_DIRECTORY))
    {
        cvarManagerGlobal->log("Could not find directory for personal bests");
        return;
    }

    //Get list of records from file
    std::vector<SequenceBestList> Records = GetRecords();  

    //Add the new record to the list (if it exists)
    PersonalBest NewPersonalBest = PersonalBest(TimerClass(CompletedTime), SkippedSteps);
    AddNewPersonalBest(Records, SequenceName, NewPersonalBest);

    //Write the file with the new record
    WriteToFile(Records);
}

std::vector<SequenceBestList> PersonalBestsManager::GetRecords()
{
    std::vector<SequenceBestList> Records;

    std::ifstream InFile(DEFAULT_BESTS_DIRECTORY / PERSONAL_BESTS_NAME);
    if(InFile)
    {
        std::string Line;
        SequenceBestList ReadSequence("");
        bool bLineIsSequenceTitle = true;
        while(!InFile.eof())
        {
            getline(InFile, Line);

            //If the current line is empty, the next line will be a sequence title
            //Push the current ReadSequence into the list
            if(Line.empty())
            {
                bLineIsSequenceTitle = true;
                continue;
            }

            //Push the old SequenceBestList object and fill the new one
            if(bLineIsSequenceTitle)
            {
                if(!ReadSequence.GetSequenceName().empty())
                {
                    Records.push_back(ReadSequence);
                }
                ReadSequence = SequenceBestList(Line);
                bLineIsSequenceTitle = false;
            }
            else
            {
                ReadSequence.AddPersonalBest(PersonalBest(Line));
            }
        }

        //Push in the final sequence
        Records.push_back(ReadSequence);
    }

    InFile.close();

    return Records;
}

void PersonalBestsManager::AddNewPersonalBest(std::vector<SequenceBestList>& Records, const std::string& SequenceName, const PersonalBest& NewPersonalBest)
{
    bool bIsRecordInList = false;
    for(auto& Record : Records)
    {
        if(Record.GetSequenceName() == SequenceName)
        {
            bIsRecordInList = true;
            Record.AddPersonalBest(NewPersonalBest);
            break;
        }
    }

    //If the record does not exist, make one
    if(!bIsRecordInList)
    {
        SequenceBestList NewList(SequenceName);
        NewList.AddPersonalBest(NewPersonalBest);
        Records.push_back(NewList);
    }
}

void PersonalBestsManager::WriteToFile(const std::vector<SequenceBestList>& Records)
{
    std::ofstream OutFile(DEFAULT_BESTS_DIRECTORY / PERSONAL_BESTS_NAME);
    if(OutFile)
    {
        //Compile lists into one string
        std::string Output;
        for(const auto& Record : Records)
        {
            Output += Record.PrintBestsList(10) + "\n\n";
        }

        //Remove last two empty lines
        Output.pop_back();
        Output.pop_back();

        //Write output to file
        OutFile << Output;
    }
    else
    {
        cvarManagerGlobal->log("Could not write to personal bests file. Invalid file?");
    }
    OutFile.close();
}

std::vector<PersonalBest> PersonalBestsManager::GetBestTimesForSequence(const std::string& InSequenceName)
{
    auto Records = GetRecords();
    for(const auto& Record : Records)
    {
        if(Record.GetSequenceName() == InSequenceName)
        {
            return Record.GetSortedBests();
        }
    }

    return std::vector<PersonalBest>();
}
