#include "SequenceManager.h"
#include "SequenceContainer.h"
#include "Sequence.h"
#include "NestedSequence.h"
#include "IndividualSequence.h"
#include "Checkpoints/Checkpoint.h"
#include "Checkpoints/DemoCarCheckpoint.h"
#include "PersonalBests/PersonalBestsManager.h"
#include "RenderingTools.h"

using namespace std::chrono;

SequenceManager::SequenceManager(std::shared_ptr<GameWrapper> InGameWrapper)
    : gameWrapper(InGameWrapper),
    SequenceData(std::make_shared<SequenceContainer>()),
    CurrentSequence(nullptr),
    PersonalBestManager(std::make_shared<PersonalBestsManager>(InGameWrapper))
{}

void SequenceManager::StartSequence(const std::string& InSequenceName, bool bResetNestedSequenceStep)
{
    //Reset the sequence
    EndSequence(false);
    PersonalBestsForSequence.clear();
    bSuccessfullyCompleted = false;
    bPendingNextSubsequence = false;
    SkippedSteps = 0;
    Timer.ResetTimer();

    //Choose whether to restart the current subsequence, or the whole nested sequence
    if(bResetNestedSequenceStep)
    {
        CurrentNestedSequenceStep = 0;
    }

    //Get the CurrentSequence either directly, or from the NestedSequence subsequence index
    CurrentMainSequenceName = InSequenceName;
    CurrentMainSequence = SequenceData->GetSequence(InSequenceName);
    if(CurrentMainSequence)
    {
        if(CurrentMainSequence->bIsNested)
        {
            //Cast main sequence to nested sequence and get its individual sequence at the currentNestedSequenceStep
            auto ThisNest = std::static_pointer_cast<NestedSequence>(CurrentMainSequence);
            CurrentSequence = ThisNest->GetSubsequence(CurrentNestedSequenceStep);
        }
        else
        {
            CurrentSequence = std::static_pointer_cast<IndividualSequence>(CurrentMainSequence);
        }
    }

    StartCurrentSequence();
}

void SequenceManager::StartCurrentSequence()
{
    if(!CurrentSequence)
    {
        return;
    }

    CurrentSequenceName = CurrentSequence->Name;
    ResetAllCheckpoints();
    PersonalBestsForSequence = PersonalBestManager->GetBestTimesForSequence(CurrentSequenceName);

    //Apply the starting properties
    CarWrapper Car = gameWrapper->GetLocalCar();
    if(!Car.IsNull())
    {
        const SequenceProperties& Properties = CurrentSequence->GetProperties();
            
        if(Properties.bSetStartBoost)
        {
            BoostWrapper Boost = Car.GetBoostComponent();
            if(!Boost.IsNull())
            {
                Boost.SetCurrentBoostAmount(Properties.StartBoostAmount / 100.f);
            }
        }

        if(Properties.bSetStartPosition)
        {
            Car.SetVelocity(Vector(0, 0, 0));
            Car.SetLocation(Properties.StartPosition);
            CarLine.LastLocation = Properties.StartPosition;
            CarLine.CurrentLocation = Properties.StartPosition;
        }

        if(Properties.bSetStartRotation)
        {
            Car.SetAngularVelocity(Vector(0, 0, 0), false);
            Car.SetCarRotation(Properties.StartRotation);
        }
    }

    //Start the sequence
    bIsSequenceActive = true;
    bEnabled = true;
}

void SequenceManager::EndSequence(bool bCompleted)
{
    if(bCompleted)
    {
        bSuccessfullyCompleted = true;
        TimeCompleted = steady_clock::now();

        //If this is a new personal best, write it in the list
        PersonalBestManager->WriteRecord(CurrentSequenceName, Timer.GetTime(), SkippedSteps);
    }

    //Update the list of best times
    PersonalBestsForSequence = PersonalBestManager->GetBestTimesForSequence(CurrentSequenceName);

    ClearAllBots();
    bIsSequenceActive = false;
    bIsTimerActive = false;
    CurrentSequence = nullptr;
    CurrentSequenceName = "";
    CurrentSequenceStep = 0;
    Timer.StopTimer();
}

void SequenceManager::ResetAllCheckpoints()
{
    //Start from a clean slate
    if(CurrentSequence)
    {
        CurrentSequence->ResetAllCheckpoints();
    }
}

void SequenceManager::ClearAllBots()
{
    //Find all the bot controllers
    ServerWrapper Server = GetCurrentGameState();
    if(Server.IsNull()) { return; }
    std::vector<ControllerWrapper> BotsToRemove;
    ArrayWrapper<ControllerWrapper> Players = Server.GetPlayers();
    for(int i = 0; i < Players.Count(); ++i)
    {
        ControllerWrapper ThisPlayer = Players.Get(i);
        if(!ThisPlayer.IsNull())
        {
            PlayerReplicationInfoWrapper PRI = ThisPlayer.GetPlayerReplicationInfo();
            if(!PRI.IsNull() && PRI.GetbBot())
            {
                BotsToRemove.push_back(ThisPlayer);
            }
        }
    }

    //Remove all the bot controllers
    for(auto& Bot : BotsToRemove)
    {
        Server.RemovePlayer(Bot);
    }
}

void SequenceManager::TryNextSubsequence(bool bEndCurrentSequence)
{
    if(!bPendingNextSubsequence)
    {
        return;
    }

    if(bEndCurrentSequence && IsSequenceActive())
    {
        EndSequence(false);
    }

    //Try loading the next subsequence
    if(CurrentMainSequence)
    {
        if(CurrentMainSequence->bIsNested)
        {
            ++CurrentNestedSequenceStep;

            //Check if there is another subsequence to go to
            auto ThisNest = std::static_pointer_cast<NestedSequence>(CurrentMainSequence);
            if(ThisNest->GetSubsequence(CurrentNestedSequenceStep))
            {
                StartSequence(CurrentMainSequenceName, false);
            }
        }
    }
}

void SequenceManager::OnGoalScored()
{
    if(IsSequenceActive())
    {
        SkippedSteps += (static_cast<int>(CurrentSequence->GetSequenceSize()) - CurrentSequenceStep - 1);
        EndSequence(true);
    }
}

void SequenceManager::Disable()
{
    bEnabled = false;
    bPendingNextSubsequence = false;
    CurrentMainSequence = nullptr;
    CurrentMainSequenceName = "";
    CurrentNestedSequenceStep = 0;
    SkippedSteps = 0;
    EndSequence(false);
}

void SequenceManager::LoadSequencesInFolder(std::filesystem::path InDirectory)
{
    SequenceData = std::make_shared<SequenceContainer>();
    SequenceData->LoadAllSequences(InDirectory);
}

const std::vector<std::string>& SequenceManager::GetSequenceFilenames()
{
    return SequenceData->GetSequenceFilenames();
}

void SequenceManager::TickSequence(CanvasWrapper Canvas, ServerWrapper Server)
{
    if(!bEnabled)
    {
        return;
    }

    if(gameWrapper->IsPaused())
    {
        //Pause timer if it is running
        if(Timer.GetbTimerRunning() && bIsTimerActive)
        {
            Timer.StopTimer();
        }
    }
    else
    {
        //Unpause timer if it is paused
        if(!Timer.GetbTimerRunning() && bIsTimerActive)
        {
            Timer.StartTimer();
        }

        //Clear checkpoints here before RenderCheckpoints may or may not run
        CheckpointNames.clear();

        //Handle rendering and collision checking
        if(CurrentSequence && bIsSequenceActive)
        {
            ClampMaxBoost();
            CheckCollisions(Server);
            RenderCheckpoints(Canvas);
        }

        //Render the HUD after RenderCheckpoints has filled the CheckpointNames vector
        RenderHUD(Canvas);
    }
}

void SequenceManager::ClampMaxBoost()
{
    const SequenceProperties& Properties = CurrentSequence->GetProperties();
    if(!Properties.bSetMaxBoost) { return; }
    CarWrapper Car = gameWrapper->GetLocalCar();
    if(Car.IsNull()) { return; }
    BoostWrapper Boost = Car.GetBoostComponent();
    if(Boost.IsNull()) { return; }

    float NewMax = Properties.MaxBoostAmount / 100.f;
    Boost.SetCurrentBoostAmount(min(Boost.GetCurrentBoostAmount(), NewMax));
}

void SequenceManager::CheckCollisions(ServerWrapper Server)
{
    if(!CurrentSequence)
    {
        return;
    }

    //Get ball
    if(Server.IsNull()) return;
    BallWrapper Ball = Server.GetBall();
    if(Ball.IsNull()) return;

    //Get car locations
    CarWrapper LocalCar = gameWrapper->GetLocalCar();
    if(LocalCar.IsNull()) return;
    PriWrapper PRI = LocalCar.GetPRI();
    if(PRI.IsNull()) return;

    //Since this is used for the current tick and the last tick, use a static variable
    CarLine.PlayerID = PRI.GetUniqueIdWrapper();
    CarLine.LastLocation = CarLine.CurrentLocation;
    CarLine.CurrentLocation = LocalCar.GetLocation();

    //Loop through all the visible checkpoints and check for collisions
    bool bIsBallActive = false;
    for(int i = 0; i < NumCheckpointsToDisplay; ++i)
    {
        if(CurrentSequenceStep + i < static_cast<int>(CurrentSequence->GetSequenceSize()))
        {
            if(auto ThisCheckpoint = CurrentSequence->GetCheckpoint(CurrentSequenceStep + i))
            {
                //Check if the ball is the one of the current checkpoints
                if(ThisCheckpoint->GetName() == "Ball")
                {
                    bIsBallActive = true;
                }

                //Control what happens with the DemoCar checkpoint type
                if(ThisCheckpoint->GetLocationType() == ELocationType::LT_DEMO_CAR)
                {
                    auto ThisDemoCar = std::static_pointer_cast<DemoCarCheckpoint>(ThisCheckpoint);
                    
                    //Spawn the car if it hasnt been spawned yet
                    if(!ThisDemoCar->IsCheckpointRevealed())
                    {
                        ThisDemoCar->SpawnCar(Server);
                    }

                    //Lock the car in the specified position and rotation
                    ThisDemoCar->SetSpawnedCarTransform(Server);
                }

                //Check if the car is intersecting with the checkpoint
                if(ThisCheckpoint->CheckCollision(LocalCar, CarLine))
                {
                    //Start timer on first checkpoint collision
                    if(!bIsTimerActive)
                    {
                        bIsTimerActive = true;
                        Timer.ResetTimer();
                        Timer.StartTimer();
                    }

                    //Increment to next sequence step
                    CurrentSequenceStep += (i + 1);
                    SkippedSteps += i;
                    if(CurrentSequenceStep >= CurrentSequence->GetSequenceSize())
                    {
                        EndSequence(true);
                        bPendingNextSubsequence = true;
                        gameWrapper->SetTimeout(std::bind(&SequenceManager::TryNextSubsequence, this, false), NextSequenceDelay);
                    }

                    //Already collided. Don't need to check any more steps
                    break;
                }
            }
        }
    }

    //Lock the ball if it isn't one of the current checkpoints
    if(!bIsBallActive)
    {
        LockBallPosition(Ball);
    }
}

void SequenceManager::LockBallPosition(BallWrapper Ball)
{
    Ball.SetLocation(Vector(0, 0, 93.2f));
    Ball.SetRotation(Rotator(0,0,0));
    Ball.SetVelocity(Vector(0,0,0));
    Ball.SetAngularVelocity(Vector(0,0,0), false);
}

void SequenceManager::RenderCheckpoints(CanvasWrapper Canvas)
{
    if(!CurrentSequence)
    {
        return;
    }

    float AnimTime = clock() / 1000.f;

    CameraWrapper Camera = gameWrapper->GetCamera();
    if(Camera.IsNull()) return;
    RT::Frustum Frustum(Canvas, Camera);

    //Loop through every checkpoint in the sequence and set them all to "unseen"
    //This is for auto-demolishing the DemoCars if they were skipped
    for(int i = 0; i < static_cast<int>(CurrentSequence->GetSequenceSize()); ++i)
    {
        if(auto ThisCheckpoint = CurrentSequence->GetCheckpoint(i))
        {
            ThisCheckpoint->SetSeen(false);
        }
    }

    //Need to draw from the back to the front.
    //If drawing from front to back, and if the next point and another later point are the same,
    //the later point would draw on top of the next point
    for(int i = NumCheckpointsToDisplay - 1; i >= 0; --i)
    {
        if(CurrentSequenceStep + i < static_cast<int>(CurrentSequence->GetSequenceSize()))
        {
            if(auto ThisCheckpoint = CurrentSequence->GetCheckpoint(CurrentSequenceStep + i))
            {
                ThisCheckpoint->SetSeen(true);

                float ColorPerc = static_cast<float>(i) / (NumCheckpointsToDisplay - 1);
                float ColorOpacity = (i == 0) ? 1.f : 0.33f;
                Canvas.SetColor(RT::GetPercentageColor(1 - ColorPerc, ColorOpacity));
                
                ThisCheckpoint->Draw(Canvas, Camera, Frustum, AnimTime);
                CheckpointNames.push_back(ThisCheckpoint->GetName());
            }
        }
    }

    //Loop through every checkpoint in the sequence and terminate them if they weren't seen
    //Really only useful for DemoCars so they can blow up the car if the checkpoint was skipped
    for(int i = 0; i < static_cast<int>(CurrentSequence->GetSequenceSize()); ++i)
    {
        auto ThisCheckpoint = CurrentSequence->GetCheckpoint(i);
        if(ThisCheckpoint && !ThisCheckpoint->GetSeen())
        {
            //Only reset the checkpoint on a DemoCar if it was previously seen
            if(ThisCheckpoint->GetLocationType() == ELocationType::LT_DEMO_CAR)
            {
                auto ThisDemoCar = std::static_pointer_cast<DemoCarCheckpoint>(ThisCheckpoint);
                if(ThisDemoCar->IsCheckpointRevealed())
                {
                    ThisCheckpoint->ResetCheckpoint();
                }
            }
        }
    }
}

void SequenceManager::RenderHUD(CanvasWrapper Canvas)
{
    //Draw the number of skipped steps above the clock
    if(SkippedSteps > 0)
    {
        Canvas.SetColor(LinearColor{255, 0, 0, 255});
        std::string SkippedText = "Skipped " + std::to_string(SkippedSteps) + " checkpoints";
        Vector2 CanvasSize = Canvas.GetSize();
        Vector2F SkippedSize = Canvas.GetStringSize(SkippedText, 3, 3);
        Vector2 SkippedPosition = {(CanvasSize.X / 2) - ((int)SkippedSize.X / 2), 90};
        Canvas.SetPosition(SkippedPosition);
        Canvas.DrawString(SkippedText, 3, 3, true);
    }

    //Draw the clock
    Timer.DrawTimer(Canvas);

    //Draw best times
    if(bShowBestTimes)
    {
        static const Vector2F MaxTimeSize = Canvas.GetStringSize("00:00:00 Skipped:100", 2, 2);
        Vector2 TextBase = {Canvas.GetSize().X - static_cast<int>(MaxTimeSize.X) - 30, 250};
        Canvas.SetColor(LinearColor{255,255,255,255});

        Canvas.SetPosition(TextBase);
        Canvas.DrawString("BEST TIMES", 3, 3, true);
        TextBase.Y += 50;

        for(const auto& BestTime : PersonalBestsForSequence)
        {
            Canvas.SetPosition(TextBase);
            Canvas.DrawString(BestTime.PrintPersonalBest(), 2, 2, true);
            TextBase.Y += 30;
        }
    }

    //Draw the completed text below the clock
    if(bSuccessfullyCompleted)
    {
        float CompletedDur = duration_cast<duration<float>>(steady_clock::now() - TimeCompleted).count();
        if(CompletedDur > 5.f)
        {
            bSuccessfullyCompleted = false;
        }

        Canvas.SetColor(LinearColor{0, 255, 0, 255});
        static const std::string CompletedText = "Completed!";
        Vector2 CanvasSize = Canvas.GetSize();
        Vector2F CompletedSize = Canvas.GetStringSize(CompletedText, 5, 5);
        Vector2 CompletedPosition = {(CanvasSize.X / 2) - ((int)CompletedSize.X / 2), 180};
        Canvas.SetPosition(CompletedPosition);
        Canvas.DrawString(CompletedText, 5, 5, true);
    }

    //Draw active sequence HUD
    if(CurrentSequence)
    {
        if(bShowCheckpointNames)
        {
            //Draw the sequence title
            Vector2 TextBase = {30, 250};
            Canvas.SetColor(LinearColor{255,255,255,255});
            Canvas.SetPosition(TextBase);
            Canvas.DrawString(CurrentSequence->Name, 3, 3, true);
            TextBase.Y += 50;

            //Draw the sequence names in their respective colors
            int CheckpointNamesSize = static_cast<int>(CheckpointNames.size());
            for(int i = CheckpointNamesSize - 1; i >= 0; --i)
            {
                float ColorPerc = static_cast<float>(i) / (CheckpointNames.size() - 1);
                float ColorOpacity = (i == CheckpointNamesSize - 1) ? 1.f : 0.5f;
                Canvas.SetColor(RT::GetPercentageColor(ColorPerc, ColorOpacity));

                Canvas.SetPosition(TextBase);
                Canvas.DrawString(CheckpointNames[i], 2, 2, true);
                TextBase.Y += 30;
            }
        }
    }
}

ServerWrapper SequenceManager::GetCurrentGameState()
{
    if(gameWrapper->IsInReplay())
        return gameWrapper->GetGameEventAsReplay().memory_address;
    else if(gameWrapper->IsInOnlineGame())
        return gameWrapper->GetOnlineGame();
    else
        return gameWrapper->GetGameEventAsServer();
}
