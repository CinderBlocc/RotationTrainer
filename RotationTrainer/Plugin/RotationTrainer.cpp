#include "RotationTrainer.h"
#include "bakkesmod\wrappers\includes.h"
#include "Sequences/SequenceManager.h"

BAKKESMOD_PLUGIN(RotationTrainer, "Rotation training plugin", "1.0", PLUGINTYPE_FREEPLAY)

/*
    TO-DO

    - ADD PERSONAL BEST LIST    ***SequenceManager.cpp, EndSequence***
        - Store (in a single file) a list of the local player's current bests for any sequence they've completed
        - Give it a .bests extension lul, it'll deter people from editing it themselves, and the file will be named personal.bests
        - Store the top 5 times for each sequence?
    
    - Pressing start doesn't stop timer    ***SequenceTimer.cpp***
        - Use GameWrapper::IsPaused(). Would only work in freeplay or when a LAN host uses the admin pause feature
        - Using accumulated deltas will fix the problem of time jumps after unpausing
    
    - Skipping checkpoints    ***SequenceManager.cpp***
        - If you hit orange or red, skip sequence to that next checkpoint, and when you finish the set it tells you how many you missed. Could be toggleable.
        - Display a red "Skipped X Checkpoints" below the clock. Add that to the record after the sequence is done
            - Sort by time in the file, but show how many checkpoints were skipped next to the time

    - CUSTOM LOCATIONS    ***SequenceContainer.cpp, SequenceManager.cpp (for spawning demo car)***
    - Have a demoable car in opponents net as a location to hit. Use LocationType::LT_DEMO_CAR
        - Formatting: DEMOCAR(X Y Z) <ROTATION(P Y R (in degrees))>
    - Have more locations players can go. Use LocationType::LT_CUSTOM_LOCATION or LocationType::LT_BOOST_SETTER - BoostSetter is an extension of CustomLocation
        - Could implement this (along with boost setter mentioned above) as just a custom location with radius specification
            - Formatting: CUSTOM(X Y Z) <RADIUS(float)> <BOOSTSET(int)> - brackets indicate optional value

    - ***RotationTrainer.cpp, SequenceManager.cpp?***
    - If a nested sequence's subsequence finishes with a ball, start the next subsequence when the goal replay is done
        - If it ends with a normal checkpoint, use the delay value before starting the next subsequence
*/

std::shared_ptr<CVarManagerWrapper> cvarManagerGlobal;

void RotationTrainer::onLoad()
{
    cvarManagerGlobal = cvarManager;

    //Create SequencesManager and load all the files in the default directory
    SequencesManager = std::make_shared<SequenceManager>(gameWrapper);
    SequencesManager->LoadSequencesInFolder(DEFAULT_CONFIG_DIRECTORY);

    bEnabled = std::make_shared<bool>(false);
    SequenceName = std::make_shared<std::string>("");
    NextSequenceDelay = std::make_shared<float>(0.f);
    cvarManager->registerCvar(CVAR_ENABLED, "1", "Enables rendering of checkpoints in rotation training plugin", true, true, 0, true, 1).bindTo(bEnabled);
    cvarManager->registerCvar(CVAR_SEQUENCE_NAME, "_Example", "Choose which sequence file to read", true).bindTo(SequenceName);
    cvarManager->registerCvar(CVAR_NEXT_SEQUENCE_DELAY, "3", "Delay between subsequences", true, true, 0, true, 120).bindTo(NextSequenceDelay);
    cvarManager->getCvar(CVAR_NEXT_SEQUENCE_DELAY).addOnValueChanged(std::bind(&RotationTrainer::OnDelayChanged, this));

    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_LOADALL,   [this](std::vector<std::string> params){ReloadSequenceFiles();}, "Load all sequences", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_START,     [this](std::vector<std::string> params){StartSequence();}, std::string("Start sequence chosen in") + CVAR_SEQUENCE_NAME, PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_END,       [this](std::vector<std::string> params){TerminateSequence();}, "End current sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_PREVIOUS,  [this](std::vector<std::string> params){/*PreviousOrNextSequence(true);*/}, "Start previous sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_NEXT,      [this](std::vector<std::string> params){/*PreviousOrNextSequence(false);*/}, "Start next sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_GET_START_INFO,     [this](std::vector<std::string> params){GetStartPointInfo();}, "Log current car info to console to use for starting point", PERMISSION_ALL);

    gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RotationTrainer::TryNextSubsequence, this));
    gameWrapper->HookEventPost("Function TAGame.GameInfo_TA.PlayerResetTraining", std::bind(&RotationTrainer::RestartSequence, this));

    gameWrapper->RegisterDrawable(std::bind(&RotationTrainer::Tick, this, std::placeholders::_1));

    GenerateSettingsFile();
}
void RotationTrainer::onUnload() {}


// SEQUENCE CONTROL //
void RotationTrainer::StartSequence()
{
    if(*bEnabled && gameWrapper->IsInFreeplay())
    {
        SequencesManager->StartSequence(*SequenceName);
    }
}

void RotationTrainer::EndSequence()
{
    SequencesManager->EndSequence();
}

void RotationTrainer::TryNextSubsequence()
{
    //Ends the current sequence and tries starting the next subsequence if it is a nested sequence
    if(SequencesManager->IsSequenceActive())
    {
        SequencesManager->EndSequence();
        SequencesManager->TryNextSubsequence();
    }
}

void RotationTrainer::OnDelayChanged()
{
    SequencesManager->SetNextSequenceDelay(cvarManager->getCvar(CVAR_NEXT_SEQUENCE_DELAY).getFloatValue());
}

void RotationTrainer::TerminateSequence()
{
    //Completely disables sequence
    //Cannot restart with "reset training" button, need to manually start sequence again
    SequencesManager->Disable();
}

void RotationTrainer::RestartSequence()
{
    //Restart sequence on "reset training" button pressed
    if(SequencesManager->IsSequenceActive())
    {
        gameWrapper->SetTimeout(std::bind(&RotationTrainer::StartSequence, this), .001f);
    }
}

void RotationTrainer::ReloadSequenceFiles()
{
    //Make sure no sequences are currently running
    TerminateSequence();

    //Reload the files and update the dropdown menu in the UI
    SequencesManager->LoadSequencesInFolder(DEFAULT_CONFIG_DIRECTORY);
    GenerateSettingsFile();
}


// TICK //
void RotationTrainer::Tick(CanvasWrapper Canvas)
{
    if(*bEnabled && gameWrapper->IsInFreeplay())
    {
        //Handles both the collision checking and rendering
        SequencesManager->TickSequence(Canvas, GetCurrentGameState());
    }
}


// MISCELLANEOUS //
ServerWrapper RotationTrainer::GetCurrentGameState()
{
    if(gameWrapper->IsInReplay())
        return gameWrapper->GetGameEventAsReplay().memory_address;
    else if(gameWrapper->IsInOnlineGame())
        return gameWrapper->GetOnlineGame();
    else
        return gameWrapper->GetGameEventAsServer();
}

void RotationTrainer::GetStartPointInfo()
{
    //Prints local car's location and rotation to console
    //Use this to get accurate "start position" and "start rotation" values for a sequence

    CarWrapper car = gameWrapper->GetLocalCar();
    if(car.IsNull())
    {
        cvarManager->log("No local car available, can't log info");
        return;
    }

    Vector loc = car.GetLocation();
    Rotator rot = car.GetRotation();

    //Generate output
    std::string output;
    output += "position " + std::to_string(loc.X) + " " + std::to_string(loc.Y) + " " + std::to_string(loc.Z) + "\n";
    output += "rotation " + std::to_string(rot.Pitch / CONST_DegToUnrRot) + " " + std::to_string(rot.Yaw / CONST_DegToUnrRot) + " " + std::to_string(rot.Roll / CONST_DegToUnrRot);

    //Copy output to the clipboard
    OpenClipboard(nullptr);
    EmptyClipboard();
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, output.size());
    if(!hg)
    {
        CloseClipboard();
        return;
    }
    memcpy(GlobalLock(hg), output.c_str(), output.size());
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
    GlobalFree(hg);

    //Print to console
    cvarManager->log("CAR START POINT INFO\n\n" + output + '\n');
    cvarManager->log(" ---> The above information has been copied to your clipboard for easy pasting.");
}
