#include "SequenceContainer.h"
#include "Checkpoints/Checkpoint.h"
#include "Checkpoints/BackboardCheckpoint.h"
#include "Checkpoints/BallCheckpoint.h"
#include "Checkpoints/BoostCheckpoints.h"
#include "Checkpoints/DemoCarCheckpoint.h"
#include "Checkpoints/PostCheckpoint.h"
#include "Sequences/SequenceProperties.h"
#include "Sequences/PendingNestedSequence.h"
#include "Sequences/IndividualSequence.h"
#include "Sequences/NestedSequence.h"
#include "Sequences/Sequence.h"
#include "utils/parser.h"
#include <filesystem>
#include <iterator>
#include <sstream>

SequenceContainer::SequenceContainer()
{
    FillDefaultCheckpoints();
}

std::shared_ptr<Sequence> SequenceContainer::GetSequence(const std::string& InSequenceName)
{
    for(const auto& Seq : AllSequences)
    {
        if(Seq->Name == InSequenceName)
        {
            return Seq;
        }
    }

    return nullptr;
}

void SequenceContainer::LoadAllSequences(std::filesystem::path InDirectory)
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

const std::vector<std::string>& SequenceContainer::GetSequenceFilenames()
{
    return AllSequenceFileNames;
}

void SequenceContainer::GetAllSequenceFiles(std::filesystem::path InDirectory)
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

SequenceProperties SequenceContainer::GetSequenceProperties(std::ifstream& InFile, bool& OutbIsNestedSequence)
{
    SequenceProperties OutputProperties;

    bool bFoundProperties = false;
    bool bFoundSequence = false;

    std::string Line;
    while(!InFile.eof())
    {
        //Search for the "begin properties" or "begin sequence" notice
        getline(InFile, Line);

        //Use # for comments
        if(!Line.empty() && Line.at(0) == '#')
        {
            continue;
        }

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

void SequenceContainer::AddIndividualSequence(std::ifstream& InFile, const std::string& Filename, SequenceProperties Properties)
{
    std::shared_ptr<IndividualSequence> ThisSequence = std::make_shared<IndividualSequence>(Filename, Properties);

    std::string LineOfText;
    while(!InFile.eof())
    {
        getline(InFile, LineOfText);

        //Use # for comments
        if(!LineOfText.empty() && LineOfText.at(0) == '#')
        {
            continue;
        }

        //Capitalize all letters in string
        for(char& c : LineOfText) { c = toupper(c); }

        //Check if the line matches one of the checkpoint codes
        if(auto ThisCheckpoint = FindDefaultCheckpoint(LineOfText))
        {
            ThisSequence->AddCheckpoint(ThisCheckpoint);
            continue;
        }

        //Check if the line is asking for a custom location
        if(LineOfText.find("CUSTOM") != std::string::npos)
        {
            if(auto ThisCheckpoint = MakeCustomCheckpoint(LineOfText))
            {
                ThisSequence->AddCheckpoint(ThisCheckpoint);
            }
            continue;
        }

        //Check if line is asking for a demo car
        if(LineOfText.find("DEMOCAR") != std::string::npos)
        {
            if(auto ThisCheckpoint = MakeDemoCarCheckpoint(LineOfText))
            {
                ThisSequence->AddCheckpoint(ThisCheckpoint);
            }
            continue;
        }
    }

    if(Properties.bUsesBall)
    {
        ThisSequence->AddCheckpoint(FindDefaultCheckpoint("Ball"));
    }

    AllSequences.push_back(ThisSequence);
}


// Nested sequences //
void SequenceContainer::AddPendingNestedSequence(std::ifstream& InFile, const std::string& Filename)
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

void SequenceContainer::CompletePendingNestedSequenceRequests()
{
    //Convert all pending nested sequences into a full nested sequence
    for(const auto& PendingSequence : AllPendingNestedSequences)
    {
        AllSequences.push_back(PendingSequence->CreateNestedSequence(AllSequences));
    }

    //Clear the list of pending sequences
    AllPendingNestedSequences.clear();
}


// Custom checkpoints //
std::shared_ptr<Checkpoint> SequenceContainer::MakeCustomCheckpoint(const std::string& InLine)
{
    //Format: CUSTOM(X, Y, Z) <RADIUS(float)> <BOOSTSET(int)> - brackets indicate optional value
    //Values do not need to be in any particular order

    //At a minimum the location must be provided
    std::vector<float> LocationVals = GetValuesFromParameter(InLine, "CUSTOM");
    if(LocationVals.size() < 3)
    {
        return nullptr;
    }

    //Create default values for the checkpoint
    Vector SpecifiedLocation = {LocationVals.at(0), LocationVals.at(1), LocationVals.at(2)};
    float SpecifiedRadius = 200.f;
    int SpecifiedBoostSet = -1;

    //Get radius
    std::vector<float> RadiusVal = GetValuesFromParameter(InLine, "RADIUS");
    if(!RadiusVal.empty())
    {
        SpecifiedRadius = RadiusVal.at(0);
    }

    //Get boost set amount
    std::vector<float> BoostSetVal = GetValuesFromParameter(InLine, "BOOSTSET");
    if(!BoostSetVal.empty())
    {
        SpecifiedBoostSet = static_cast<int>(BoostSetVal.at(0));
    }

    //Get the name of this checkpoint. Defaults to UNNAMED LOCATION
    std::string SpecifiedName = GetNameFromCustomCheckpoint(InLine, false);

    //Create the checkpoint if at minimum the location was provided
    return std::make_shared<LocationCheckpoint>(SpecifiedName, SpecifiedLocation, SpecifiedRadius, SpecifiedBoostSet);
}

std::shared_ptr<Checkpoint> SequenceContainer::MakeDemoCarCheckpoint(const std::string& InLine)
{    
    //Format: DEMOCAR(X, Y, Z) <ROTATION(P, Y, R (in degrees))> - brackets indicate optional value
    //Values do not need to be in any particular order
    
    //At a minimum the location must be provided
    std::vector<float> LocationVals = GetValuesFromParameter(InLine, "DEMOCAR");
    if(LocationVals.size() < 3)
    {
        return nullptr;
    }

    //Create default values for the checkpoint
    Vector SpecifiedLocation = {LocationVals.at(0), LocationVals.at(1), LocationVals.at(2)};
    Rotator SpecifiedRotation = {0, 0, 0};

    //Get rotation
    std::vector<float> RotationVals = GetValuesFromParameter(InLine, "ROTATION");
    if(RotationVals.size() >= 3)
    {
        SpecifiedRotation =
        {
            static_cast<int>(RotationVals.at(0) * CONST_DegToUnrRot),
            static_cast<int>(RotationVals.at(1) * CONST_DegToUnrRot),
            static_cast<int>(RotationVals.at(2) * CONST_DegToUnrRot)
        };
    }

    //Get the name of this checkpoint. Defaults to UNNAMED DEMO CAR
    std::string SpecifiedName = GetNameFromCustomCheckpoint(InLine, true);

    //Create the checkpoint if at minimum the location was provided
    return std::make_shared<DemoCarCheckpoint>(SpecifiedName, SpecifiedLocation, SpecifiedRotation);
}

std::vector<float> SequenceContainer::GetValuesFromParameter(const std::string& LineToSearch, const std::string& SearchTerm)
{
    std::vector<float> Output;

    // SEARCHTERM(var, var, var, var, etc) //

    //Look for search term in line of text
    size_t SearchResult = LineToSearch.find(SearchTerm);
    if(SearchResult != std::string::npos)
    {
        //Get the first opening parentheses after SearchTerm
        size_t SearchResultEnd = SearchResult + SearchTerm.size();
        if(SearchResultEnd < LineToSearch.size() && LineToSearch.at(SearchResultEnd) == '(')
        {
            //Find the first closing parentheses
            size_t CloseParenthesesPos = LineToSearch.find(')', SearchResultEnd);
            if(CloseParenthesesPos != std::string::npos)
            {
                size_t ContentsSize = CloseParenthesesPos - SearchResultEnd;
                std::string Contents = LineToSearch.substr(SearchResultEnd + 1, ContentsSize - 1);
                std::vector<std::string> SplitStrings;
                split(Contents, SplitStrings, ',');

                //Convert the comma separated values into floats
                for(const auto& SplitString : SplitStrings)
                {
                    try
                    {
                        Output.push_back(stof(SplitString));
                    }
                    catch(...)
                    {
                    	cvarManagerGlobal->log("stof threw an exception on line: " + LineToSearch);
                    }
                }
            }
        }
    }

    return Output;
}

std::string SequenceContainer::GetNameFromCustomCheckpoint(const std::string& LineToSearch, bool bIsDemoCar)
{
    std::string Output = bIsDemoCar ? "UNNAMED DEMO CAR" : "UNNAMED LOCATION";

    //Look for search term in line of text
    static const std::string SearchTerm = "NAME";
    size_t SearchResult = LineToSearch.find(SearchTerm);
    if(SearchResult != std::string::npos)
    {
        //Get the first opening parentheses after SearchTerm
        size_t SearchResultEnd = SearchResult + SearchTerm.size();
        if(SearchResultEnd < LineToSearch.size() && LineToSearch.at(SearchResultEnd) == '(')
        {
            //Find the first closing parentheses
            size_t CloseParenthesesPos = LineToSearch.find(')', SearchResultEnd);
            if(CloseParenthesesPos != std::string::npos)
            {
                size_t ContentsSize = CloseParenthesesPos - SearchResultEnd;
                Output = LineToSearch.substr(SearchResultEnd + 1, ContentsSize - 1);
            }
        }
    }

    return Output;
}


// List of default checkpoints //
void SequenceContainer::FillDefaultCheckpoints()
{
    //Backboard
    DefaultCheckpoints.push_back(std::make_shared<BackboardCheckpoint>(Vector{ 955.f, 5120.f, 128.f}, Vector{ 2800.f, 5120.f, 700.f}, EAnimationDirection::ANIM_CW, "BL", ELocation::LOC_BACKBOARD_LEFT,  "Backboard Left"));
    DefaultCheckpoints.push_back(std::make_shared<BackboardCheckpoint>(Vector{-955.f, 5120.f, 128.f}, Vector{-2800.f, 5120.f, 700.f}, EAnimationDirection::ANIM_CW, "BR", ELocation::LOC_BACKBOARD_RIGHT, "Backboard Right"));

    //Post
    DefaultCheckpoints.push_back(std::make_shared<PostCheckpoint>(Vector{ 955.f, -5120.f, 10.f}, Vector{ 2500.f, -5120.f, 1450.f}, EAnimationDirection::ANIM_CW,  "PL", ELocation::LOC_POST_LEFT,  "Post Left"));
    DefaultCheckpoints.push_back(std::make_shared<PostCheckpoint>(Vector{-955.f, -5120.f, 10.f}, Vector{-2500.f, -5120.f, 1450.f}, EAnimationDirection::ANIM_CCW, "PR", ELocation::LOC_POST_RIGHT, "Post Right"));

    //Large Boosts
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3072.f, -4096.f, 73.f}, "BLBL", ELocation::LOC_LBOOST_BACK_LEFT,      "Boost Large Back Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3072.f, -4096.f, 73.f}, "BLBR", ELocation::LOC_LBOOST_BACK_RIGHT,     "Boost Large Back Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3584.f,  0.f,    73.f}, "BLML", ELocation::LOC_LBOOST_MIDFIELD_LEFT,  "Boost Large Midfield Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3584.f,  0.f,    73.f}, "BLMR", ELocation::LOC_LBOOST_MIDFIELD_RIGHT, "Boost Large Midfield Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3072.f,  4096.f, 73.f}, "BLFL", ELocation::LOC_LBOOST_FRONT_LEFT,     "Boost Large Front Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3072.f,  4096.f, 73.f}, "BLFR", ELocation::LOC_LBOOST_FRONT_RIGHT,    "Boost Large Front Right"));

    //Small Boosts
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1792.f, -4184.f, 70.f}, "BSBGLL", ELocation::LOC_SBOOST_BACK_GOALLINE_LEFT,        "Boost Small Back Goal Line Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -4240.f, 70.f}, "BSBGLC", ELocation::LOC_SBOOST_BACK_GOALLINE_CENTER,      "Boost Small Back Goal Line Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1792.f, -4184.f, 70.f}, "BSBGLR", ELocation::LOC_SBOOST_BACK_GOALLINE_RIGHT,       "Boost Small Back Goal Line Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 940.f,  -3308.f, 70.f}, "BSBGBL", ELocation::LOC_SBOOST_BACK_GOALBOX_LEFT,         "Boost Small Back Goal Box Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -2816.f, 70.f}, "BSBGBC", ELocation::LOC_SBOOST_BACK_GOALBOX_CENTER,       "Boost Small Back Goal Box Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-940.f,  -3308.f, 70.f}, "BSBGBR", ELocation::LOC_SBOOST_BACK_GOALBOX_RIGHT,        "Boost Small Back Goal Box Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 3584.f, -2484.f, 70.f}, "BSBMFL", ELocation::LOC_SBOOST_BACK_MIDDLE_FAR_LEFT,      "Boost Small Back Middle Far Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1788.f, -2300.f, 70.f}, "BSBMCL", ELocation::LOC_SBOOST_BACK_MIDDLE_CENTER_LEFT,   "Boost Small Back Middle Center Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1788.f, -2300.f, 70.f}, "BSBMCR", ELocation::LOC_SBOOST_BACK_MIDDLE_CENTER_RIGHT,  "Boost Small Back Middle Center Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-3584.f, -2484.f, 70.f}, "BSBMFR", ELocation::LOC_SBOOST_BACK_MIDDLE_FAR_RIGHT,     "Boost Small Back Middle Far Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 2048.f, -1036.f, 70.f}, "BSBML",  ELocation::LOC_SBOOST_BACK_MIDFIELD_LEFT,        "Boost Small Back Midfield Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -1024.f, 70.f}, "BSBMC",  ELocation::LOC_SBOOST_BACK_MIDFIELD_CENTER,      "Boost Small Back Midfield Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-2048.f, -1036.f, 70.f}, "BSBMR",  ELocation::LOC_SBOOST_BACK_MIDFIELD_RIGHT,       "Boost Small Back Midfield Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1024.f,  0.f,    70.f}, "BSML",   ELocation::LOC_SBOOST_MIDFIELD_LEFT,             "Boost Small Midfield Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1024.f,  0.f,    70.f}, "BSMR",   ELocation::LOC_SBOOST_MIDFIELD_RIGHT,            "Boost Small Midfield Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-2048.f,  1036.f, 70.f}, "BSFMR",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_RIGHT,      "Boost Small Front Midfield Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     1024.f, 70.f}, "BSFMC",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_CENTER,     "Boost Small Front Midfield Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 2048.f,  1036.f, 70.f}, "BSFML",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_LEFT,       "Boost Small Front Midfield Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-3584.f,  2484.f, 70.f}, "BSFMFR", ELocation::LOC_SBOOST_FRONT_MIDDLE_FAR_RIGHT,    "Boost Small Front Middle Far Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1788.f,  2300.f, 70.f}, "BSFMCR", ELocation::LOC_SBOOST_FRONT_MIDDLE_CENTER_RIGHT, "Boost Small Front Middle Center Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1788.f,  2300.f, 70.f}, "BSFMCL", ELocation::LOC_SBOOST_FRONT_MIDDLE_CENTER_LEFT,  "Boost Small Front Middle Center Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 3584.f,  2484.f, 70.f}, "BSFMFL", ELocation::LOC_SBOOST_FRONT_MIDDLE_FAR_LEFT,     "Boost Small Front Middle Far Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-940.f,   3308.f, 70.f}, "BSFGBR", ELocation::LOC_SBOOST_FRONT_GOALBOX_RIGHT,       "Boost Small Front Goal Box Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     2816.f, 70.f}, "BSFGBC", ELocation::LOC_SBOOST_FRONT_GOALBOX_CENTER,      "Boost Small Front Goal Box Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 940.f,   3308.f, 70.f}, "BSFGBL", ELocation::LOC_SBOOST_FRONT_GOALBOX_LEFT,        "Boost Small Front Goal Box Left"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1792.f,  4184.f, 70.f}, "BSFGLR", ELocation::LOC_SBOOST_FRONT_GOALLINE_RIGHT,      "Boost Small Front Goal Line Right"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     4240.f, 70.f}, "BSFGLC", ELocation::LOC_SBOOST_FRONT_GOALLINE_CENTER,     "Boost Small Front Goal Line Center"));
    DefaultCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1792.f,  4184.f, 70.f}, "BSFGLL", ELocation::LOC_SBOOST_FRONT_GOALLINE_LEFT,       "Boost Small Front Goal Line Left"));

    //Ball
    DefaultCheckpoints.push_back(std::make_shared<BallCheckpoint>("Ball"));
}

std::shared_ptr<Checkpoint> SequenceContainer::FindDefaultCheckpoint(const std::string& CheckpointName)
{
    for(const auto& ThisCheckpoint : DefaultCheckpoints)
    {
        if(ThisCheckpoint->GetCode() == CheckpointName)
        {
            return ThisCheckpoint;
        }
    }

    return nullptr;
}
