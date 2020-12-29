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
    std::shared_ptr<SequenceManager> SequencesManager;

	//Cvars
	std::shared_ptr<bool> bEnabled;
	std::shared_ptr<std::string> SequenceName;

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
