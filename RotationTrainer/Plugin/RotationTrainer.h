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
	std::shared_ptr<std::string> SequenceName;
    std::shared_ptr<float> NextSequenceDelay;

public:
	void onLoad() override;
	void onUnload() override;

    //Sequence Control
	void StartSequence();
	void EndSequence();
    void TryNextSubsequence();
    void OnDelayChanged();
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
