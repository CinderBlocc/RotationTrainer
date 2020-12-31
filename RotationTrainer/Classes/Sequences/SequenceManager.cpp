#include "SequenceManager.h"
#include "SequenceContainer.h"
#include "MacrosStructsEnums.h"
#include "Sequence.h"
#include "NestedSequence.h"
#include "IndividualSequence.h"
#include "Checkpoints/Checkpoint.h"
#include "RenderingTools.h"

/*

    Handle personal bests writing in EndSequence?
        If you do that, you'll have to ensure the bests are only saved if the sequence was completed
        Don't want people skipping through sequences and getting insane bests


    Need to do the DemoCar reveal check and SpawnCar thing somewhere


    Need to check collisions in all the checkpoints that are being rendered

*/
using namespace std::chrono;

SequenceManager::SequenceManager(std::shared_ptr<GameWrapper> InGameWrapper)
    : gameWrapper(InGameWrapper), SequenceData(std::make_shared<SequenceContainer>()), CurrentSequence(nullptr) {}

void SequenceManager::StartSequence(const std::string& InSequenceName, bool bResetNestedSequenceStep)
{
    //Reset the sequence
    EndSequence(false);
    bSuccessfullyCompleted = false;
    bPendingNextSubsequence = false;
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
    bIsSequenceActive = false;
    bIsTimerActive = false;
    CurrentSequence = nullptr;
    CurrentSequenceName = "";
    CurrentSequenceStep = 0;
    Timer.StopTimer();

    if(bCompleted)
    {
        bSuccessfullyCompleted = true;
        TimeCompleted = steady_clock::now();
    }

    /*
        
        Handle personal bests here if player completed the whole sequence
    
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
}

void SequenceManager::ResetAllCheckpoints()
{
    //Start from a clean slate
    if(CurrentSequence)
    {
        CurrentSequence->ResetAllCheckpoints();
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
    if(CurrentMainSequence && CurrentMainSequence->bIsNested)
    {
        ++CurrentNestedSequenceStep;
        StartSequence(CurrentMainSequenceName, false);
    }
}

void SequenceManager::Disable()
{
    bEnabled = false;
    bPendingNextSubsequence = false;
    CurrentMainSequence = nullptr;
    CurrentMainSequenceName = "";
    CurrentNestedSequenceStep = 0;
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
    static CarLocations CarLine;
    CarLine.PlayerID = PRI.GetUniqueIdWrapper();
    CarLine.LastLocation = CarLine.CurrentLocation;
    CarLine.CurrentLocation = LocalCar.GetLocation();

    auto ThisCheckpoint = CurrentSequence->GetCheckpoint(CurrentSequenceStep);
    if(ThisCheckpoint)
    {
        //Freeze the ball unless it is the current target
        if(ThisCheckpoint->GetName() != "Ball")
        {
            LockBallPosition(Ball);
        }

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
            ++CurrentSequenceStep;
            if(CurrentSequenceStep >= CurrentSequence->GetSequenceSize())
            {
                EndSequence(true);
                bPendingNextSubsequence = true;
                gameWrapper->SetTimeout(std::bind(&SequenceManager::TryNextSubsequence, this, false), NextSequenceDelay);
            }
        }
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

    //Need to draw from the back to the front.
    //If drawing from front to back, and if the next point and the third point are the same,
    //the third point would draw on top of the next point
    for(int i = NumCheckpointsToDisplay - 1; i >= 0; --i)
    {
        if(CurrentSequenceStep + i < static_cast<int>(CurrentSequence->GetSequenceSize()))
        {
            if(auto ThisCheckpoint = CurrentSequence->GetCheckpoint(CurrentSequenceStep + i))
            {
                float ColorPerc = static_cast<float>(i) / (NumCheckpointsToDisplay - 1);
                float ColorOpacity = (i == 0) ? 1.f : 0.33f;
                Canvas.SetColor(RT::GetPercentageColor(1 - ColorPerc, ColorOpacity));
                
                ThisCheckpoint->Draw(Canvas, Camera, Frustum, AnimTime);
                CheckpointNames.push_back(ThisCheckpoint->GetName());
            }
        }
    }
}

void SequenceManager::RenderHUD(CanvasWrapper Canvas)
{
    //Draw the clock
    Timer.DrawTimer(Canvas);

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
