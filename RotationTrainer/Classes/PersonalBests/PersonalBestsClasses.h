#pragma once
#include <string>

class TimerClass
{
public:
    TimerClass(float InTime);
    TimerClass(const std::string& InString);

    std::string PrintTime();
    
private:
    int Minutes;
    int Seconds;
    int Milliseconds;

    TimerClass() = delete;
    std::string FormatClockNumber(int InNum);

    //Comparison operators starting with Minutes, then Seconds, then Milliseconds
};

class PersonalBest
{
public:
    PersonalBest(TimerClass InTimer, int InSkippedSteps);
    PersonalBest(const std::string& InString);

    std::string PrintPersonalBest();

private:
    TimerClass Time;
    int SkippedSteps;

    PersonalBest() = delete;

    //Comparison operators starting with Time, then SkippedSteps
};
