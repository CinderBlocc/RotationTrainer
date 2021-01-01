#include "RotationTrainer.h"
#include "bakkesmod\wrappers\includes.h"
#include "Sequences/SequenceManager.h"

BAKKESMOD_PLUGIN(RotationTrainer, "Rotation training plugin", "1.0", PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> cvarManagerGlobal;
std::shared_ptr<GameWrapper> gameWrapperGlobal;

// PLUGIN BOILERPLATE //
void RotationTrainer::onLoad()
{
    cvarManagerGlobal = cvarManager;
    gameWrapperGlobal = gameWrapper;

    using namespace std::placeholders;

    //Create SequencesManager and load all the files in the default directory
    SequencesManager = std::make_shared<SequenceManager>(gameWrapper);
    SequencesManager->LoadSequencesInFolder(DEFAULT_CONFIG_DIRECTORY);

    bEnabled = std::make_shared<bool>(false);
    bShowCheckpointNames = std::make_shared<bool>(false);
    bShowBestTimes = std::make_shared<bool>(false);
    SequenceName = std::make_shared<std::string>("");
    NextSequenceDelay = std::make_shared<float>(0.f);
    cvarManager->registerCvar(CVAR_ENABLED, "1", "Enables rendering of checkpoints in rotation training plugin", true, true, 0, true, 1).bindTo(bEnabled);
    cvarManager->registerCvar(CVAR_SHOW_CHECKPOINT_NAMES, "1", "Displays the list of upcoming checkpoints", true, true, 0, true, 1).bindTo(bShowCheckpointNames);
    cvarManager->registerCvar(CVAR_SHOW_BEST_TIMES, "0", "Displays the list of best times for the current sequence", true, true, 0, true, 1).bindTo(bShowBestTimes);
    cvarManager->registerCvar(CVAR_SEQUENCE_NAME, "_Example", "Choose which sequence file to read", true).bindTo(SequenceName);
    cvarManager->registerCvar(CVAR_NEXT_SEQUENCE_DELAY, "3", "Delay between subsequences", true, true, 0, true, 120).bindTo(NextSequenceDelay);
    cvarManager->getCvar(CVAR_NEXT_SEQUENCE_DELAY).addOnValueChanged(std::bind(&RotationTrainer::OnDelayChanged, this));
    cvarManager->getCvar(CVAR_SHOW_CHECKPOINT_NAMES).addOnValueChanged(std::bind(&RotationTrainer::OnShowCheckpointsChanged, this));
    cvarManager->getCvar(CVAR_SHOW_BEST_TIMES).addOnValueChanged(std::bind(&RotationTrainer::OnShowBestTimesChanged, this));

    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_LOADALL,   [this](std::vector<std::string> params){ReloadSequenceFiles();}, "Load all sequences", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_START,     [this](std::vector<std::string> params){StartSequence();}, std::string("Start sequence chosen in") + CVAR_SEQUENCE_NAME, PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_END,       [this](std::vector<std::string> params){TerminateSequence();}, "End current sequence", PERMISSION_ALL);
  //cvarManager->registerNotifier(NOTIFIER_SEQUENCE_PREVIOUS,  [this](std::vector<std::string> params){/*PreviousOrNextSequence(true);*/}, "Start previous sequence", PERMISSION_ALL);
  //cvarManager->registerNotifier(NOTIFIER_SEQUENCE_NEXT,      [this](std::vector<std::string> params){/*PreviousOrNextSequence(false);*/}, "Start next sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_GET_START_INFO,     [this](std::vector<std::string> params){GetStartPointInfo();}, "Log current car info to console to use for starting point", PERMISSION_ALL);    
    
    gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RotationTrainer::OnGoalScored, this));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.StartNewRound", std::bind(&RotationTrainer::OnNextRoundStarted, this));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&RotationTrainer::TerminateSequence, this));
    gameWrapper->HookEventPost("Function TAGame.GameInfo_TA.PlayerResetTraining", std::bind(&RotationTrainer::RestartSequence, this));
    gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.EventDemolished", std::bind(&RotationTrainer::OnCarDemolished, this, _1, _2, _3));

    gameWrapper->RegisterDrawable(std::bind(&RotationTrainer::Tick, this, std::placeholders::_1));

    GenerateSettingsFile();
}
void RotationTrainer::onUnload() {}


// SEQUENCE CONTROL //
void RotationTrainer::StartSequence(bool bResetNestedSequenceStep)
{
    if(IsValidMode())
    {
        bPendingNextSubsequence = false;
        SequencesManager->StartSequence(*SequenceName, bResetNestedSequenceStep);
    }
}

void RotationTrainer::EndSequence(bool bCompleted)
{
    SequencesManager->EndSequence(bCompleted);
}

void RotationTrainer::TryNextSubsequence()
{
    if(SequencesManager->IsSequenceActive())
    {
        SequencesManager->EndSequence(false);
    }
    
    if(IsValidMode())
    {
        SequencesManager->SetPendingNextSubsequence(true);
        SequencesManager->TryNextSubsequence();
    }
}

void RotationTrainer::TerminateSequence()
{
    //Completely disables sequence
    //Cannot restart with "reset training" button, need to manually start sequence again
    SequencesManager->Disable();
}

void RotationTrainer::RestartSequence()
{
    //Restart current sequence on "reset training" button pressed
    if(SequencesManager->IsEnabled())
    {
        gameWrapper->SetTimeout(std::bind(&RotationTrainer::StartSequence, this, false), .001f);
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

void RotationTrainer::OnShowCheckpointsChanged()
{
    SequencesManager->SetShowCheckpointNames(cvarManager->getCvar(CVAR_SHOW_CHECKPOINT_NAMES).getBoolValue());
}

void RotationTrainer::OnShowBestTimesChanged()
{
    SequencesManager->SetShowBestTimes(cvarManager->getCvar(CVAR_SHOW_BEST_TIMES).getBoolValue());
}

void RotationTrainer::OnDelayChanged()
{
    //This only works for sequences that end with a checkpoint and not with the ball
    SequencesManager->SetNextSequenceDelay(cvarManager->getCvar(CVAR_NEXT_SEQUENCE_DELAY).getFloatValue());
}

void RotationTrainer::OnGoalScored()
{
    SequencesManager->OnGoalScored();

    //Queue up the next subsequence to start when the goal replay is done
    if(IsValidMode())
    {
        bPendingNextSubsequence = true;
    }
}

void RotationTrainer::OnNextRoundStarted()
{
    //Delay the start of the next sequence until the next tick so the game can get set up
    gameWrapper->SetTimeout([this](GameWrapper* gw)
    {
        if(IsValidMode() && bPendingNextSubsequence)
        {
            TryNextSubsequence();
            bPendingNextSubsequence = false;
        }
    }, 0.001f);
}

void RotationTrainer::OnCarDemolished(CarWrapper Car, void* params, std::string funcName)
{
    if(!IsValidMode())
    {
        return;
    }

    //Determine if the car is a bot
    if(Car.IsNull()) { return; }
    PriWrapper PRI = Car.GetPRI();
    if(PRI.IsNull()) { return; }
    if(PRI.GetbBot())
    {
        //Remove the bot from the lobby so it doesn't respawn
        AIControllerWrapper AIController = Car.GetAIController();
        if(AIController.IsNull()) { return; }
        ServerWrapper Server = GetCurrentGameState();
        if(Server.IsNull()) { return; }

        Server.RemovePlayer(AIController);
    }
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

bool RotationTrainer::IsValidMode()
{
    return *bEnabled && gameWrapper->IsInFreeplay();
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
