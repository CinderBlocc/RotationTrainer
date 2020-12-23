#include "SequenceManager.h"
#include "Checkpoints/Checkpoint.h" // <-- probably need to import each type of checkpoint
#include "Sequences/SequenceProperties.h"
#include "Sequences/PendingNestedSequence.h"
#include "Sequences/IndividualSequence.h"
#include "Sequences/NestedSequence.h"
#include "Sequences/Sequence.h"
#include <filesystem>
#include <iterator>
#include <sstream>

//Stores a pointer to the list of checkpoints stored in the plugin so that it can be used to fill individual sequences
SequenceManager::SequenceManager(std::vector<std::shared_ptr<Checkpoint>>* InCheckpoints) : AllCheckpoints(InCheckpoints) {}


void SequenceManager::LoadAllSequences(std::filesystem::path InDirectory)
{
    //Clean out the current sequences
    AllSequences.clear();

    //Get all the files that contain sequences
    GetAllSequenceFiles(InDirectory);

    //Fill the vector with individual sequences
    //If it is a nested sequence, create a pending request
    for(const auto& SequenceFileName : AllSequenceFileNames)
    {
        std::ifstream InFile = std::ifstream(InDirectory / std::string(SequenceFileName + ".cfg"));
        bool bIsNestedSequence = false;

        //Get the sequence properties
        SequenceProperties Properties = GetSequenceProperties(InFile, bIsNestedSequence);

        //Failed to find sequence indicator. Skip to the next file
        if(InFile.eof())
        {
            InFile.close();
            continue;
        }

        //Choose the type of sequence to build
        if(bIsNestedSequence)
        {
            AddPendingNestedSequence(InFile, SequenceFileName);
        }
        else
        {
            AddIndividualSequence(InFile, SequenceFileName, Properties);
        }

        InFile.close();
    }

    //Once the individual sequences are loaded, fill the nested sequences with the correct pointers
    CompletePendingNestedSequenceRequests();
}

const std::vector<std::string>& SequenceManager::GetSequenceFilenames()
{
    return AllSequenceFileNames;
}

void SequenceManager::GetAllSequenceFiles(std::filesystem::path InDirectory)
{
    //Loop through every file in the sequences folder, and store the filenames of the ones that are to be used
    
    //Clear all references to files
    AllSequenceFileNames.clear();

    std::error_code ec;
    if(!std::filesystem::exists(InDirectory, ec))
    {
        return;
    }

    //Loop through all files in the ColorChanger data folder to find config files
    for(const auto &entry : std::filesystem::directory_iterator(InDirectory))
    {
        if(entry.path().extension().u8string() == ".cfg")
        {
            std::string name = entry.path().stem().u8string();
            AllSequenceFileNames.push_back(name);
        }
    }
}

SequenceProperties SequenceManager::GetSequenceProperties(std::ifstream& InFile, bool& OutbIsNestedSequence)
{
    SequenceProperties OutputProperties;

    bool bFoundProperties = false;
    bool bFoundSequence = false;

    std::string Line;
    while(!InFile.eof())
    {
        //Search for the "begin properties" or "begin sequence" notice
        getline(InFile, Line);

        //Capitalize all letters in string
        for(char& c : Line) { c = toupper(c); }

        if(Line == "BEGIN PROPERTIES")
        {
            //Properties section has started
            bFoundProperties = true;
            continue;
        }
        if(Line == "BEGIN SEQUENCE")
        {
            //Normal sequence. Properties are done, exit function
            bFoundSequence = true;
            OutbIsNestedSequence = false;
            break;
        }
        if(Line == "BEGIN SEQUENCE LIST")
        {
            //Nested sequence. Properties are done, exit function
            bFoundSequence = true;
            OutbIsNestedSequence = true;
            break;
        }

        //Properties section has been found, search for individual properties
        if(bFoundProperties)
        {
            if(Line == "NO BALL")
            {
                OutputProperties.bUsesBall = false;
            }

            //Put strings into a stream for iterations
            std::stringstream LineStream(Line);
            std::istream_iterator<std::string> IterBegin(LineStream), IterEnd;
            std::vector<std::string> SplitStrings(IterBegin, IterEnd);

            if(SplitStrings.size() >= 3 && SplitStrings[0] == "BOOST")
            {
                //[0]BOOST [1]START [2]<value>
                if(SplitStrings[1] == "START")
                {
                    OutputProperties.bSetStartBoost   = true;
                    OutputProperties.StartBoostAmount = stoi(SplitStrings[2]);
                }

                //[0]BOOST [1]MAX [2]<value>
                if(SplitStrings[1] == "MAX")
                {
                    OutputProperties.bSetMaxBoost   = true;
                    OutputProperties.MaxBoostAmount = stoi(SplitStrings[2]);
                }
            }

            if(SplitStrings.size() >= 4)
            {
                //[0]POSITION [1]<X> [2]<Y> [3]<Z>
                if(SplitStrings[0] == "POSITION")
                {
                    OutputProperties.bSetStartPosition = true;
                    OutputProperties.StartPosition.X   = stof(SplitStrings[1]);
                    OutputProperties.StartPosition.Y   = stof(SplitStrings[2]);
                    OutputProperties.StartPosition.Z   = stof(SplitStrings[3]);
                }

                //[0]ROTATION [1]<Pitch> [2]<Yaw> [3]<Roll>
                if(SplitStrings[0] == "ROTATION")
                {
                    OutputProperties.bSetStartRotation   = true;
                    OutputProperties.StartRotation.Pitch = static_cast<int>(stof(SplitStrings[1]) * CONST_DegToUnrRot);
                    OutputProperties.StartRotation.Yaw   = static_cast<int>(stof(SplitStrings[2]) * CONST_DegToUnrRot);
                    OutputProperties.StartRotation.Roll  = static_cast<int>(stof(SplitStrings[3]) * CONST_DegToUnrRot);
                }
            }
        }
    }

    return OutputProperties;
}

void SequenceManager::AddIndividualSequence(std::ifstream& InFile, const std::string& Filename, SequenceProperties Properties)
{
    std::shared_ptr<IndividualSequence> ThisSequence = std::make_shared<IndividualSequence>(Filename, Properties);

    std::string CheckpointName;
    while(!InFile.eof())
    {
        getline(InFile, CheckpointName);
        if(auto ThisCheckpoint = FindCheckpoint(CheckpointName))
        {
            ThisSequence->AddCheckpoint(ThisCheckpoint);
        }
    }

    AllSequences.push_back(ThisSequence);
}

std::shared_ptr<Checkpoint> SequenceManager::FindCheckpoint(std::string CheckpointName)
{
    for(const auto& ThisCheckpoint : *AllCheckpoints)
    {
        if(ThisCheckpoint->GetName() == CheckpointName)
        {
            return ThisCheckpoint;
        }
    }

    return nullptr;
}

void SequenceManager::AddPendingNestedSequence(std::ifstream& InFile, const std::string& Filename)
{
    std::shared_ptr<PendingNestedSequence> ThisPendingSequence = std::make_shared<PendingNestedSequence>(Filename);

    std::string SequenceName;
    while(!InFile.eof())
    {
        getline(InFile, SequenceName);
        ThisPendingSequence->AddSequenceRequest(SequenceName);
    }

    AllPendingNestedSequences.push_back(ThisPendingSequence);
}

void SequenceManager::CompletePendingNestedSequenceRequests()
{
    //Convert all pending nested sequences into a full nested sequence
    for(const auto& PendingSequence : AllPendingNestedSequences)
    {
        AllSequences.push_back(PendingSequence->CreateNestedSequence(AllSequences));
    }

    //Clear the list of pending sequences
    AllPendingNestedSequences.clear();
}
