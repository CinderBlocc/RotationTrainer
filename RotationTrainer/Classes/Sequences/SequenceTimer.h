#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <chrono>

class SequenceTimer
{
public:
    void StartTimer();
    void StopTimer();
    void ResetTimer();

    bool GetbTimerRunning();
    float GetTime();

    void DrawTimer(CanvasWrapper Canvas);

private:
    std::chrono::steady_clock::time_point StartTime;
    float StoppedTime = 0;
    bool bIsTimerRunning = false;

    std::string FormatClockNumber(int InNum);
};
