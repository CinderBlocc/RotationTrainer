#include "SequenceTimer.h"

void SequenceTimer::StartTimer()
{
    PreviousTime = std::chrono::steady_clock::now();
    bIsTimerRunning = true;
}

void SequenceTimer::StopTimer()
{
    bIsTimerRunning = false;
}

void SequenceTimer::ResetTimer()
{
    Time = 0;
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
        auto CurrentTime = steady_clock::now();
        float Delta = duration_cast<duration<float>>(CurrentTime - PreviousTime).count();
        PreviousTime = CurrentTime;

        Time += Delta;
    }

    return Time;
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
