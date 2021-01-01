#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/wrapperstructs.h"
#include "bakkesmod/wrappers/UniqueIDWrapper.h"
#include <vector>
#include <memory>




//Temporary definition to block broken DemoCar functionality.
//Current issue: once a bot is spawned, it can't be removed.
//Might be fixed in a future SDK update that would add RemovePlayer function to ServerWrapper.
//#define NO_DEMO_CAR




extern std::shared_ptr<CVarManagerWrapper> cvarManagerGlobal;
extern std::shared_ptr<GameWrapper> gameWrapperGlobal;


// MACROS //
#define CVAR_ENABLED               "Rotation_Enabled"
#define CVAR_SHOW_CHECKPOINT_NAMES "Rotation_ShowCheckpointNames"
#define CVAR_SHOW_BEST_TIMES       "Rotation_ShowBestTimes"
#define CVAR_SEQUENCE_NAME         "Rotation_SequenceName"
#define CVAR_NEXT_SEQUENCE_DELAY   "Rotation_NextSequenceDelay"
#define NOTIFIER_SEQUENCE_LOADALL  "Rotation_LoadAllSequences"
#define NOTIFIER_SEQUENCE_START    "Rotation_StartSequence"
#define NOTIFIER_SEQUENCE_END      "Rotation_EndSequence"
#define NOTIFIER_SEQUENCE_PREVIOUS "Rotation_PreviousSequence"
#define NOTIFIER_SEQUENCE_NEXT     "Rotation_NextSequence"
#define NOTIFIER_GET_START_INFO    "Rotation_GetStartPointInfo"
#define PERSONAL_BESTS_NAME        "Personal.Bests"
#define DEFAULT_BESTS_DIRECTORY    gameWrapper->GetDataFolder() / "RotationTrainer"
#define DEFAULT_CONFIG_DIRECTORY   DEFAULT_BESTS_DIRECTORY / "Sequences"
#define DEMOCAR_BASE_NAME          "Demo Car"


// ENUMS //
enum class EAnimationDirection
{
	ANIM_DEFAULT = 0,
	ANIM_CW,
	ANIM_CCW,
	ANIM_MAX
};

enum class ELocationType
{
	LT_DEFAULT = 0,
	LT_LARGE_BOOST,
	LT_SMALL_BOOST,
	LT_BACKBOARD,
	LT_POST,
	LT_BALL,
    LT_DEMO_CAR,
    LT_CUSTOM_LOCATION,
	LT_MAX
};

enum class ELocation
{
	LOC_DEFAULT = 0,
	
	//Large Boosts
	LOC_LBOOST_BACK_LEFT,
	LOC_LBOOST_BACK_RIGHT,
	LOC_LBOOST_MIDFIELD_LEFT,
	LOC_LBOOST_MIDFIELD_RIGHT,
	LOC_LBOOST_FRONT_LEFT,
	LOC_LBOOST_FRONT_RIGHT,
	
	//Small Boosts
	LOC_SBOOST_BACK_GOALLINE_LEFT,
	LOC_SBOOST_BACK_GOALLINE_CENTER,
	LOC_SBOOST_BACK_GOALLINE_RIGHT,
	LOC_SBOOST_BACK_GOALBOX_LEFT,
	LOC_SBOOST_BACK_GOALBOX_CENTER,
	LOC_SBOOST_BACK_GOALBOX_RIGHT,
	LOC_SBOOST_BACK_MIDDLE_FAR_LEFT,
	LOC_SBOOST_BACK_MIDDLE_CENTER_LEFT,
	LOC_SBOOST_BACK_MIDDLE_CENTER_RIGHT,
	LOC_SBOOST_BACK_MIDDLE_FAR_RIGHT,
	LOC_SBOOST_BACK_MIDFIELD_LEFT,
	LOC_SBOOST_BACK_MIDFIELD_CENTER,
	LOC_SBOOST_BACK_MIDFIELD_RIGHT,
	LOC_SBOOST_MIDFIELD_LEFT,
	LOC_SBOOST_MIDFIELD_RIGHT,
	LOC_SBOOST_FRONT_MIDFIELD_RIGHT,
	LOC_SBOOST_FRONT_MIDFIELD_CENTER,
	LOC_SBOOST_FRONT_MIDFIELD_LEFT,
	LOC_SBOOST_FRONT_MIDDLE_FAR_RIGHT,
	LOC_SBOOST_FRONT_MIDDLE_CENTER_RIGHT,
	LOC_SBOOST_FRONT_MIDDLE_CENTER_LEFT,
	LOC_SBOOST_FRONT_MIDDLE_FAR_LEFT,
	LOC_SBOOST_FRONT_GOALBOX_RIGHT,
	LOC_SBOOST_FRONT_GOALBOX_CENTER,
	LOC_SBOOST_FRONT_GOALBOX_LEFT,
	LOC_SBOOST_FRONT_GOALLINE_RIGHT,
	LOC_SBOOST_FRONT_GOALLINE_CENTER,
	LOC_SBOOST_FRONT_GOALLINE_LEFT,

	//Backboard
	LOC_BACKBOARD_LEFT,
	LOC_BACKBOARD_RIGHT,

	//Post
	LOC_POST_LEFT,
	LOC_POST_RIGHT,

	//Ball
	LOC_BALL,

	LOC_MAX
};

enum class ELoadSequenceError
{
    SEQERR_ISALREADYSEQUENCELIST = 0,
    SEQERR_ISALREADYCHECKPOINTSLIST
};


// STRUCTS //
struct CarLocations
{
	Vector LastLocation;
	Vector CurrentLocation;
	UniqueIDWrapper PlayerID;
};
