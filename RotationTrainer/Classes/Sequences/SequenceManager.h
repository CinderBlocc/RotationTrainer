#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "SequenceTimer.h"

class SequenceContainer;
class Sequence;
class IndividualSequence;

class SequenceManager
{
public:
    SequenceManager(std::shared_ptr<GameWrapper> InGameWrapper);

    //Gameplay handling
    void StartSequence(const std::string& InSequenceName);
    void EndSequence();
    void TryNextSubsequence();
    void Disable();

    bool IsSequenceActive() { return bIsSequenceActive; }

    //Rendering and collision
    void TickSequence(CanvasWrapper Canvas, ServerWrapper Server);
    void SetNumCheckpointsToDisplay(int NewNumCheckpoints) { NumCheckpointsToDisplay = NewNumCheckpoints; }

    //Info about which sequences are ready to go
    void LoadSequencesInFolder(std::filesystem::path InDirectory);
    const std::vector<std::string>& GetSequenceFilenames();

private:
    SequenceManager() = delete;

    std::shared_ptr<GameWrapper> gameWrapper;

    std::shared_ptr<SequenceContainer> SequenceData;
    std::shared_ptr<Sequence> CurrentMainSequence; //Could be a nested sequence. If individual, it will be the same as CurrentSequence
    std::shared_ptr<IndividualSequence> CurrentSequence;

    SequenceTimer Timer;

    bool bEnabled = false;
    bool bIsSequenceActive = false;
    int CurrentSequenceStep = 0;
    int CurrentNestedSequenceStep = 0;
    int NumCheckpointsToDisplay = 5;

    void CheckCollisions(ServerWrapper Server);
    void RenderCheckpoints(CanvasWrapper Canvas);
    void RenderHUD(CanvasWrapper Canvas);

    void ClampMaxBoost();
    void LockBallPosition(BallWrapper Ball);

    std::vector<std::string> CheckpointNames;
};
