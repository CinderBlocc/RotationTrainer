#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Checkpoints/Checkpoint.h"
#include "MacrosStructsEnums.h"
#include "RenderingTools.h"

class SequenceManager;

class RotationTrainer : public BakkesMod::Plugin::BakkesModPlugin
{
private:
    // THE GOOD STUFF //
    std::shared_ptr<SequenceManager> SequencesManager;


    // THE BAD STUFF //
	//Globals
	LoadedSequence currentSequence;
	std::vector<LoadedSequence> sequenceList;
	size_t currentSequenceStep = 0;
	bool sequenceIsActive = false;
	bool isInSequence = false;
	bool timerRunning = false;
	std::chrono::steady_clock::time_point startTime;
	std::string clockTime;

	//Locations of all cars (current and previous tick)
	CarLocations localCarLocations;

	//Cvars
	std::shared_ptr<bool> enabled;
	std::shared_ptr<std::string> renderType;
	std::shared_ptr<std::string> SequenceName;

	//Time
	std::chrono::steady_clock::time_point previousTime;
	float seconds = 0;

public:
	void onLoad() override;
	void onUnload() override;

    //Sequence Control
	void StartSequence();
	void EndSequence();
    void TryNextSubsequence();
	void TerminateSequence();
	void RestartSequence();
    void ReloadSequenceFiles();

    //Tick
	void Tick(CanvasWrapper Canvas);

    //Miscellaneous
    ServerWrapper GetCurrentGameState();
	void GetStartPointInfo();

    //UI (SettingsFileGenerator.cpp)
	void GenerateSettingsFile();
	std::string RebuildSettingsGetCFGFiles();
};
