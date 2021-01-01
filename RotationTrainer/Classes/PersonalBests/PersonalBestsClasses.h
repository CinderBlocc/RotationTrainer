#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>


// TIMER CLASS //
// Stores a time //
class TimerClass
{
public:
    TimerClass(float InTime);
    TimerClass(const std::string& InString);

    std::string PrintTime() const;

    //Relational operators
    friend inline bool operator< (const TimerClass& lhs, const TimerClass& rhs)
    {
        return std::tie(lhs.Minutes, lhs.Seconds, lhs.Milliseconds) < std::tie(rhs.Minutes, rhs.Seconds, rhs.Milliseconds);
    }
    friend inline bool operator> (const TimerClass& lhs, const TimerClass& rhs) { return   rhs < lhs;  }
    friend inline bool operator<=(const TimerClass& lhs, const TimerClass& rhs) { return !(lhs > rhs); }
    friend inline bool operator>=(const TimerClass& lhs, const TimerClass& rhs) { return !(lhs < rhs); }
    
private:
    int Minutes;
    int Seconds;
    int Milliseconds;

    std::string FormatClockNumber(int InNum) const;

    TimerClass() = delete;
};


// PERSONAL BEST CLASS //
// Stores a single time record including skipped checkpoints //
class PersonalBest
{
public:
    PersonalBest(const std::string& InString);
    PersonalBest(TimerClass InTimer, int InSkippedSteps);

    std::string PrintPersonalBest() const;

    //Relational operators
    friend inline bool operator< (const PersonalBest& lhs, const PersonalBest& rhs)
    {
        return std::tie(lhs.SkippedSteps, lhs.Time) < std::tie(rhs.SkippedSteps, rhs.Time);
    }
    friend inline bool operator> (const PersonalBest& lhs, const PersonalBest& rhs) { return   rhs < lhs;  }
    friend inline bool operator<=(const PersonalBest& lhs, const PersonalBest& rhs) { return !(lhs > rhs); }
    friend inline bool operator>=(const PersonalBest& lhs, const PersonalBest& rhs) { return !(lhs < rhs); }

private:
    TimerClass Time;
    int SkippedSteps;

    PersonalBest() = delete;
};


// SEQUENCE BEST CLASS //
// Stores a list of the best times per sequence //
class SequenceBestList
{
public:
    SequenceBestList(const std::string& InSequenceName) : SequenceName(InSequenceName) {}

    void AddPersonalBest(const PersonalBest& Best) { Bests.push_back(Best); }
    const std::string& GetSequenceName() const { return SequenceName; }
    std::vector<PersonalBest> GetSortedBests() const;

    std::string PrintBestsList(size_t MaxRecordsToPrint = 10) const;

private:
    std::string SequenceName;
    std::vector<PersonalBest> Bests;


    SequenceBestList() = delete;
};
