#include "SequenceTimer.h"
#include "PersonalBests/PersonalBestsClasses.h"

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
    TimerClass Timer(GetTime());
    std::string ClockText = Timer.PrintTime();

    Canvas.SetColor(LinearColor{255,255,255,255});
    Vector2F ClockStringSize = Canvas.GetStringSize(ClockText, 5, 5);
    Vector2 ClockPosition = {(int)((Canvas.GetSize().X * .5f) - (ClockStringSize.X * .5f)), 125};
    Canvas.SetPosition(ClockPosition);
    Canvas.DrawString(ClockText, 5, 5, true);
}
