#include "RotationTrainer.h"
#include "Sequences/SequenceManager.h"
#include <fstream>
#include <filesystem>
#include <iterator>

#define nl(x) setFile << x << '\n'
#define blank setFile << '\n'
#define cv(x) std::string(x)

void RotationTrainer::RebuildSettingsFile()
{
    std::ofstream setFile("./bakkesmod/plugins/settings/RotationTrainer.set");

    nl("Rotation Trainer");
    nl("1|Enable plugin|" + cv(CVAR_ENABLED));
    blank;
    blank;
    nl("8|");
    blank;
    blank;
    nl("9|SEQUENCE SETTINGS");
    blank;
    nl("0|Start Current Sequence|" + cv(NOTIFIER_SEQUENCE_START));
    nl("7");
    nl("0|End Current Sequence|" + cv(NOTIFIER_SEQUENCE_END));
    nl("7");
    nl("0|Start Previous Sequence|" + cv(NOTIFIER_SEQUENCE_PREVIOUS));
    nl("7");
    nl("0|Start Next Sequence|" + cv(NOTIFIER_SEQUENCE_NEXT));
    blank;
    nl("6|Sequence Name|"  + cv(CVAR_SEQUENCE_NAME) + "|" << RebuildSettingsGetCFGFiles());
    nl("7");
    nl("0|Update sequence list|"  + cv(NOTIFIER_REBUILD_UI));
    blank;
    nl("8|");
    nl("9|SEQUENCE BUILDING TOOLS");
    nl("0|Get Car Info For Start Position/Rotation|openmenu console2; " + cv(NOTIFIER_GET_START_INFO));

    setFile.close();

    cvarManager->executeCommand("cl_settings_refreshplugins");
}

std::string RotationTrainer::RebuildSettingsGetCFGFiles()
{
    std::string Output;

    bool bFirst = true;
    for(const auto& SequenceFilename : SequencesManager->GetSequenceFilenames())
    {
        if(!bFirst)
        {
            Output += '&';
        }

        Output += SequenceFilename + '@' + SequenceFilename;

        bFirst = false;
    }

    return Output;
}