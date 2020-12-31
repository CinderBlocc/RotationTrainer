#include "PersonalBestsClasses.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "utils/parser.h"


TimerClass::TimerClass(float InTime)
    : Minutes(0), Seconds(0), Milliseconds(0)
{
    Minutes      = static_cast<int>(InTime) / 60;
    Seconds      = static_cast<int>(InTime) - (Minutes * 60);
    Milliseconds = static_cast<int>((InTime - static_cast<int>(InTime)) * 100);
}

TimerClass::TimerClass(const std::string& InString)
    : Minutes(0), Seconds(0), Milliseconds(0)
{
    std::vector<std::string> SplitStrings;
    split(InString, SplitStrings, ':');
    if(SplitStrings.size() >= 3)
    {
        Minutes = stoi(SplitStrings.at(0));
        Seconds = stoi(SplitStrings.at(1));
        Milliseconds = stoi(SplitStrings.at(2));
    }
}

std::string TimerClass::PrintTime()
{
    return FormatClockNumber(Minutes) + ":" + FormatClockNumber(Seconds) + ":" + FormatClockNumber(Milliseconds);
}

std::string TimerClass::FormatClockNumber(int InNum)
{
    std::string Output;
    if(InNum < 10)
    {
        Output += "0";
    }
    Output += std::to_string(InNum);
    
    return Output;
}



PersonalBest::PersonalBest(TimerClass InTimer, int InSkippedSteps)
    : Time(InTimer), SkippedSteps(InSkippedSteps) {}

PersonalBest::PersonalBest(const std::string& InString)
    : Time(0), SkippedSteps(0)
{
    // FORMATS
    // 12:34:56
    // 12:34:56 Skipped:1

    std::vector<std::string> TimeAndSkips;
    split(InString, TimeAndSkips, ' ');

    //Set time and leave skips to default 0
    //Skips are not printed if there were no skips
    if(TimeAndSkips.size() >= 1)
    {
        Time = TimerClass(TimeAndSkips.at(0));
    }

    //Set correct skips value if it was printed
    if(TimeAndSkips.size() >= 2)
    {
        std::vector<std::string> SkippedAndValue;
        split(TimeAndSkips.at(1), SkippedAndValue, ':');
        if(SkippedAndValue.size() >= 2)
        {
            SkippedSteps = stoi(SkippedAndValue.at(1));
        }
    }
}

std::string PersonalBest::PrintPersonalBest()
{
    std::string Output = Time.PrintTime();
    if(SkippedSteps > 0)
    {
        Output += " Skipped:" + std::to_string(SkippedSteps);
    }

    return Output;
}
