#pragma once
#pragma comment(lib, "PluginSDK.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "MacrosStructsEnums.h"

class SequenceManager;

class RotationTrainer : public BakkesMod::Plugin::BakkesModPlugin
{
private:
    std::shared_ptr<SequenceManager> SequencesManager;

	//Cvars
	std::shared_ptr<bool> bEnabled;
	std::shared_ptr<bool> bShowCheckpointNames;
	std::shared_ptr<bool> bShowBestTimes;
	std::shared_ptr<std::string> SequenceName;
    std::shared_ptr<float> NextSequenceDelay;

public:
    //Plugin Boilerplate
	void onLoad() override;
	void onUnload() override;

    //Sequence Control
    bool bPendingNextSubsequence = false;
	void StartSequence(bool bResetNestedSequenceStep = true);
	void EndSequence(bool bCompleted);
    void TryNextSubsequence();
	void TerminateSequence();
	void RestartSequence();
    void ReloadSequenceFiles();
    void OnShowCheckpointsChanged();
    void OnShowBestTimesChanged();
    void OnDelayChanged();
    void OnGoalScored();
    void OnNextRoundStarted();
    void OnCarDemolished(CarWrapper Car, void* params, std::string funcName);

    //Tick
	void Tick(CanvasWrapper Canvas);

    //Miscellaneous
    ServerWrapper GetCurrentGameState();
    bool IsValidMode();
	void GetStartPointInfo();

    //UI (SettingsFileGenerator.cpp)
	void GenerateSettingsFile();
	std::string RebuildSettingsGetCFGFiles();
};
