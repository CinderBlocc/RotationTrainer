#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include <chrono>
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
    void StartSequence(const std::string& InSequenceName, bool bResetNestedSequenceStep = true);
    void EndSequence(bool bCompleted);
    void SetPendingNextSubsequence(bool bNewValue) { bPendingNextSubsequence = bNewValue; }
    void TryNextSubsequence(bool bEndCurrentSequence = true);
    void Disable();

    bool IsEnabled() { return bEnabled; }
    bool IsSequenceActive() { return bIsSequenceActive; }
    void SetNextSequenceDelay(float InDelay) { NextSequenceDelay = InDelay; }

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
    std::shared_ptr<Sequence> CurrentMainSequence;
    std::shared_ptr<IndividualSequence> CurrentSequence;
    std::string CurrentMainSequenceName;
    std::string CurrentSequenceName;

    SequenceTimer Timer;

    bool bEnabled = false;
    bool bIsSequenceActive = false;
    bool bIsTimerActive = false;
    bool bPendingNextSubsequence = false;
    int CurrentSequenceStep = 0;
    int CurrentNestedSequenceStep = 0;
    int NumCheckpointsToDisplay = 5;
    float NextSequenceDelay = 3.f;

    bool bSuccessfullyCompleted = false;
    std::chrono::steady_clock::time_point TimeCompleted;

    void StartCurrentSequence();
    void ResetAllCheckpoints();

    void CheckCollisions(ServerWrapper Server);
    void RenderCheckpoints(CanvasWrapper Canvas);
    void RenderHUD(CanvasWrapper Canvas);

    void ClampMaxBoost();
    void LockBallPosition(BallWrapper Ball);

    std::vector<std::string> CheckpointNames;
};
