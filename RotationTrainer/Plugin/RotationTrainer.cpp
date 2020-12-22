#include "bakkesmod\wrappers\includes.h"
#include "RotationTrainer.h"

#include "Sequences/SequenceManager.h"

#include <fstream>
#include <filesystem>
#include <iterator>
#include <cctype>
#include <sstream>

using namespace std::chrono;

BAKKESMOD_PLUGIN(RotationTrainer, "Rotation training plugin", "0.9.0", PLUGINTYPE_CUSTOM_TRAINING)

/*
    TO-DO

    - ADD SEQUENCE OF SEQUENCES: Just like a normal sequence.cfg, but it's a list of other sequences that will run once one sequence has ended and the player hits reset
        - Really close to finishing this one, just re-read through everything until it makes sense and add the last bits
        - Might want to pull the sequence list reading out of the LoadSequence function and bring it into StartSequence
    
    - Move sequence properties into the LoadedSequence struct so you can properly set properties when that sequence is called from the list of sequences
    
    - ADD PERSONAL BEST LIST
        - Store (in a single file) a list of the local player's current bests for any sequence they've completed
        - Give it a .bests extension lul, it'll deter people from editing it themselves, and the file will be named personal.bests
    
    - Pressing start doesn't stop timer (not sure if this is needed).
        - Use GameWrapper::IsPaused(). Would only work in freeplay or when a LAN host uses the admin pause feature
    
    - If you hit orange or red, skip sequence to that next checkpoint, and when you finish the set it tells you how many you missed. Could be toggleable.
        - ADD A 1 SECOND PENALTY PER SKIP. When sorting personal bests, it'll be much easier to track when an objective time change is in place
            - Indicate a penalty will be added by displaying a red "+9s" below the clock. Add that to the time after the sequence is done
    
    - Set maximum boost cap to encourage flips to navigate the field. Make it a property ranging 0 - 100
        - Partially implemented. It is an available property, but it doesn't actually set anything yet

    - Have a demoable car in opponents net as a location to hit. Use LocationType::LT_DEMO_CAR
    
    - Have a location which sets boost so it can take away your boost. Use LocationType::LT_BOOST_SETTER - BoostSetter is an extension of CustomLocation
    
    - Have more locations players can go. Use LocationType::LT_CUSTOM_LOCATION
        - Could implement this (along with boost setter mentioned above) as just a custom location with radius specification
            - Formatting: LOCATION(X Y Z) <RADIUS(float)> <BOOSTSET(int)> - brackets indicate optional value
*/

void RotationTrainer::onLoad()
{
    CreateAllCheckpoints();

    //Create SequencesManager and load all the files in the default directory
    SequencesManager = std::make_shared<SequenceManager>(&AllCheckpoints);
    SequencesManager->LoadAllSequences(DEFAULT_CONFIG_DIRECTORY);

    enabled = std::make_shared<bool>(false);
    renderType = std::make_shared<std::string>("");
    sequenceName = std::make_shared<std::string>("");
    cvarManager->registerCvar(CVAR_ENABLED, "1", "Enables rendering of checkpoints in rotation training plugin", true, true, 0, true, 1).bindTo(enabled);
    cvarManager->registerCvar(CVAR_RENDER_TYPE, "ALL", "Choose which type of object to render", true).bindTo(renderType);
    cvarManager->registerCvar(CVAR_SEQUENCE_NAME, "Example", "Choose which sequence file to read", true).bindTo(sequenceName);

    cvarManager->registerNotifier(NOTIFIER_REBUILD_UI,         [this](std::vector<std::string> params){RebuildSettingsFile();}, "Rebuild UI file with list of sequences from /data/RotationTrainer/Sequences/", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_LOADALL,   [this](std::vector<std::string> params){SequencesManager->LoadAllSequences(DEFAULT_CONFIG_DIRECTORY);}, "Load all sequences", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_START,     [this](std::vector<std::string> params){StartSequence();}, std::string("Start sequence chosen in") + CVAR_SEQUENCE_NAME, PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_END,       [this](std::vector<std::string> params){TerminateSequence();}, "End current sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_PREVIOUS,  [this](std::vector<std::string> params){PreviousOrNextSequence(true);}, "Start previous sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_SEQUENCE_NEXT,      [this](std::vector<std::string> params){PreviousOrNextSequence(false);}, "Start next sequence", PERMISSION_ALL);
    cvarManager->registerNotifier(NOTIFIER_GET_START_INFO,     [this](std::vector<std::string> params){GetStartPointInfo();}, "Log current car info to console to use for starting point", PERMISSION_ALL);

    gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RotationTrainer::EndSequence, this));
    gameWrapper->HookEventPost("Function TAGame.GameInfo_TA.PlayerResetTraining", std::bind(&RotationTrainer::RestartSequence, this));

    gameWrapper->RegisterDrawable(bind(&RotationTrainer::Render, this, std::placeholders::_1));

    RebuildSettingsFile();
}
void RotationTrainer::onUnload() {}
ServerWrapper RotationTrainer::GetCurrentGameState()
{
    if(gameWrapper->IsInReplay())
        return gameWrapper->GetGameEventAsReplay().memory_address;
    else if(gameWrapper->IsInOnlineGame())
        return gameWrapper->GetOnlineGame();
    else
        return gameWrapper->GetGameEventAsServer();
}

// SEQUENCE CONTROL FUNCTIONS
void RotationTrainer::StartSequence()
{
    if(!(*enabled) || gameWrapper->IsInOnlineGame()) { return; }

    //Clear the current sequence and load the new one from the file specified by *sequenceName
    currentSequence.checkpoints.clear();
    //currentSequence = LoadSequence(*sequenceName);//Need to get this to return a vector of sequences if it determines it is a sequence list, not a checkpoint list

    //Set initial values
    ResetClockTime();
    currentSequenceStep = 0;
    sequenceIsActive = true;
    isInSequence = true;
}
void RotationTrainer::EndSequence()
{
    /*
    
    
        Personal bests here
        - Open the personal.bests file
        - Check if the current sequence's name is listed there
            - If not, add the current run to the list and close file
            - If it is, compare current run to run in file
                - If nothing improved, close file
                - If there was improvement, overwrite the listing in the file with the current run, then close the file

        DATA FOR PERSONAL BESTS - sorted by priority
        - Time
        - Locations missed/skipped
            - Maybe add a 3 second penalty for each pad skipped so that it sorts the time better
    
    
    */


    //Keeps sequence active in the background if the user wants to restart
    timerRunning = false;
    isInSequence = false;
    //currentSequenceStep = 0;
}
void RotationTrainer::TerminateSequence()
{
    //Completely disables sequence.
    //Cannot restart with "restart training" button, need to manually start sequence again

    timerRunning = false;
    sequenceIsActive = false;
    isInSequence = false;
    currentSequence.checkpoints.clear();
    currentSequenceStep = 0;
}
void RotationTrainer::RestartSequence()
{
    //Restart sequence on "restart training" button pressed
    if(!(*enabled)) { return; }

    if(sequenceIsActive)
    {
        timerRunning = false;
        ResetClockTime();
        gameWrapper->SetTimeout(std::bind(&RotationTrainer::StartSequence, this), .001f);
    }
}
void RotationTrainer::PreviousOrNextSequence(bool prevOrNext)
{
    //Take current sequence name (sequenceName) and compare it to list of files in directory alphabetically
    //If it is the last cfg in the list, wrap around to the first cfg and vice versa
    
    if(gameWrapper->IsInOnlineGame()) return;

    //Get list of all cfg files in directory
    std::vector<std::string> filenames;



    // NEED TO CHECK IF DIRECTORY EXISTS FIRST //



    for(const auto &entry : std::filesystem::directory_iterator("./bakkesmod/data/RotationTrainer/Sequences/"))
    {
        if(entry.path().extension().u8string() == ".cfg")
        {
            filenames.push_back(entry.path().stem().u8string());
        }
    }

    //Find which sequence to start
    CVarWrapper SequenceNameCvar = cvarManager->getCvar(CVAR_SEQUENCE_NAME);
    if(SequenceNameCvar.IsNull()) { return; }

    if(prevOrNext)
    {
        //Find previous sequence
        for(auto it = filenames.end() - 1; it != filenames.begin() - 1; --it)
        {
            if(*it < *sequenceName)
            {
                SequenceNameCvar.setValue(*it);
                StartSequence();
                return;
            }
        }

        //Sequence was the first in the list, wrap around
        SequenceNameCvar.setValue(*(filenames.end() - 1));
        StartSequence();
        return;
    }
    else
    {
        //Find next sequence
        for(auto &filename : filenames)
        {
            if(filename > *sequenceName)
            {
                SequenceNameCvar.setValue(filename);
                StartSequence();
                return;
            }
        }

        //Sequence was the last in the list, wrap around
        SequenceNameCvar.setValue(*(filenames.begin()));
        StartSequence();
        return;
    }
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

// TICK FUNCTIONS
void RotationTrainer::Render(CanvasWrapper canvas)
{
    GetTime();
    debugStrings.clear();
    
    if(!(*enabled) || gameWrapper->IsInOnlineGame() || !sequenceIsActive) return;

    CameraWrapper camera = gameWrapper->GetCamera();
    if(camera.IsNull()) return;
    RA.frustum = RT::Frustum(canvas, camera);

    //Get ball
    ServerWrapper server = GetCurrentGameState();
    if(server.IsNull()) return;
    BallWrapper ball = server.GetBall();
    if(ball.IsNull()) return;

    //Get car locations
    CarWrapper localCar = gameWrapper->GetLocalCar();
    if(localCar.IsNull()) return;
    PriWrapper PRI = localCar.GetPRI();
    if(PRI.IsNull()) return;
    localCarLocations.PlayerID = PRI.GetUniqueIdWrapper();
    localCarLocations.LastLocation = localCarLocations.CurrentLocation;
    localCarLocations.CurrentLocation = localCar.GetLocation();

    //Render next 3 checkpoints and check for collision on first checkpoint
    if(!currentSequence.checkpoints.empty() && isInSequence)
    {
        //Freeze the ball unless it is the current target
        if(currentSequence.checkpoints[currentSequenceStep]->GetName() != "Ball")
        {
            ball.SetLocation(Vector(0, 0, 93.150002f));
            ball.SetRotation(Rotator(0,0,0));
            ball.SetVelocity(Vector(0,0,0));
            ball.SetAngularVelocity(Vector(0,0,0), false);
        }

        bool hasHitCheckpoint = currentSequence.checkpoints[currentSequenceStep]->CheckCollision(localCar, localCarLocations);
        if(hasHitCheckpoint)
        {
            //Start timer on first checkpoint collision
            if(!timerRunning)
            {
                startTime = steady_clock::now();
                timerRunning = true;
            }

            //Increment to next sequence step
            if(currentSequenceStep + 1 >= currentSequence.checkpoints.size())
            {
                EndSequence();
            }
            else
            {
                ++currentSequenceStep;
            }
        }
        //Need to first check if currentSequence.checkpoints[currentSequenceStep + i] is in the vector's range, then check its collision
        //if(TestCollision(localCar, localCarLocations, currentSequence.checkpoints[currentSequenceStep + 1]))
        //if(TestCollision(localCar, localCarLocations, currentSequence.checkpoints[currentSequenceStep + 2]))

        DrawSequenceCheckpoints(canvas, camera);
    }

    DrawClock(canvas);

    //Draw debug strings
    while(debugStrings.size() > 45)
    {
        debugStrings.erase(debugStrings.begin());
    }
    RT::DrawDebugStrings(canvas, debugStrings, RT::EDebugStringBackground::BG_StaticWidth, 450);
}
void RotationTrainer::GetTime()
{
    auto now = steady_clock::now();
    auto diff = duration_cast<duration<double>>(now - previousTime);
    previousTime = now;
    
    seconds = clock() / 1000.f;
    delta = diff.count();
}
void RotationTrainer::ResetClockTime()
{
    clockTime = "00:00:00";
}
std::string RotationTrainer::FormatNumber(int num)
{
    std::string output;
    if(num < 10)
    {
        output += "0";
    }
    output += std::to_string(num);
    
    return output;
}
void RotationTrainer::DrawClock(CanvasWrapper canvas)
{
    if(timerRunning)
    {
        //ResetClockTime();
        float currTime = duration_cast<duration<float>>(steady_clock::now() - startTime).count();
        
        //Adding "this" to each of these because "seconds" already exists as a member variable
        int thisMinutes      = (int)currTime / 60;
        int thisSeconds      = (int)currTime - (thisMinutes * 60);
        int thisMilliseconds = (int)((currTime - (int)currTime) * 100);
        
        clockTime = FormatNumber(thisMinutes) + ":" + FormatNumber(thisSeconds) + ":" + FormatNumber(thisMilliseconds);
    }

    canvas.SetColor(LinearColor{255,255,255,255});
    Vector2 clockPos = {canvas.GetSize().X / 2 - 150, 125};
    canvas.SetPosition(clockPos);
    canvas.DrawString(clockTime, 5, 5);
}
void RotationTrainer::DrawSequenceCheckpoints(CanvasWrapper canvas,CameraWrapper camera)
{
    Vector2 base = {30, 250};

    LinearColor colors[3];
    colors[0] = LinearColor{0,255,0,255};
    colors[1] = LinearColor{255,200,0,255};
    colors[2] = LinearColor{255,0,0,255};

    //Draw the title
    canvas.SetColor(LinearColor{255,255,255,255});
    canvas.SetPosition(base);
    canvas.DrawString(*sequenceName,3,3);
    base.Y += 20;

    //Need to draw from the back to the front.
    //If drawing from front to back, and if the next point and the third point are the same,
    //the third point would draw on top of the next point
    for(int i = 2; i >= 0; --i)
    {
        if(currentSequenceStep + i < currentSequence.checkpoints.size())
        {
            canvas.SetColor(colors[i]);
            currentSequence.checkpoints[currentSequenceStep + i]->Draw(canvas, camera, RA.frustum, seconds);
            base.Y += 30;
            canvas.SetPosition(base);
            canvas.DrawString(currentSequence.checkpoints[currentSequenceStep + i]->GetName(), 2, 2);
        }
    }
}
