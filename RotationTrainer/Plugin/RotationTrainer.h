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
	RT::RenderingAssistant RA;
    std::shared_ptr<SequenceManager> SequencesManager;


    // THE BAD STUFF //
	//Globals
	std::vector<std::shared_ptr<Checkpoint>> AllCheckpoints;
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
	std::shared_ptr<std::string> sequenceName;

	//Time
	std::chrono::steady_clock::time_point previousTime;
	float seconds = 0;
	double delta = 0;
	void GetTime();
	void ResetClockTime();

	//idk
	void CreateAllCheckpoints();
	void RebuildSettingsFile();
	std::string RebuildSettingsGetCFGFiles();

	//Sequence file parsing
	//LoadedSequence LoadSequence(std::string seqName);
    //LoadedSequence SendLoadSequenceError(ELoadSequenceError error, std::string seqName);
	void StartSequence();
	void EndSequence();
	void TerminateSequence();
	void RestartSequence();
	void PreviousOrNextSequence(bool prevOrNext);
	void GetStartPointInfo();

	//Tick functions
	void Render(CanvasWrapper canvas);
	void DrawSequenceCheckpoints(CanvasWrapper canvas, CameraWrapper camera);
	void DrawClock(CanvasWrapper canvas);

	/**/ std::vector<RT::DebugString> debugStrings;

public:
	void onLoad() override;
	void onUnload() override;

	ServerWrapper GetCurrentGameState();
	std::string FormatNumber(int num);
};
