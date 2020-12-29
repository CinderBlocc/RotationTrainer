#include "SequenceTimer.h"

/*

    Need to switch the timer count storage so it accumulates a delta each tick
    This way you can pause the timer while the game is paused, and restarting it won't have a weird jump in time

*/

void SequenceTimer::StartTimer()
{
    StartTime = std::chrono::steady_clock::now();
    bIsTimerRunning = true;
}

void SequenceTimer::StopTimer()
{
    StoppedTime = GetTime();
    bIsTimerRunning = false;
}

void SequenceTimer::ResetTimer()
{
    StoppedTime = 0;
}

bool SequenceTimer::GetbTimerRunning()
{
    return bIsTimerRunning;
}

float SequenceTimer::GetTime()
{
    using namespace std::chrono;

    if(bIsTimerRunning)
    {
        return duration_cast<duration<float>>(steady_clock::now() - StartTime).count();
    }

    return StoppedTime;
}

void SequenceTimer::DrawTimer(CanvasWrapper Canvas)
{
    float CurrentTime = GetTime();
        
    int Minutes      = static_cast<int>(CurrentTime) / 60;
    int Seconds      = static_cast<int>(CurrentTime) - (Minutes * 60);
    int Milliseconds = static_cast<int>((CurrentTime - static_cast<int>(CurrentTime)) * 100);
        
    std::string ClockText = FormatClockNumber(Minutes) + ":" + FormatClockNumber(Seconds) + ":" + FormatClockNumber(Milliseconds);

    Canvas.SetColor(LinearColor{255,255,255,255});
    Vector2F ClockStringSize = Canvas.GetStringSize(ClockText, 5, 5);
    Vector2 ClockPosition = {(int)((Canvas.GetSize().X * .5f) - (ClockStringSize.X * .5f)), 125};
    Canvas.SetPosition(ClockPosition);
    Canvas.DrawString(ClockText, 5, 5, true);
}

std::string SequenceTimer::FormatClockNumber(int InNum)
{
    std::string Output;
    if(InNum < 10)
    {
        Output += "0";
    }
    Output += std::to_string(InNum);
    
    return Output;
}
