#pragma once
#include "LocationCheckpoint.h"

//BoostCheckpointLarge
class BoostCheckpointLarge : public LocationCheckpoint
{
public:
    BoostCheckpointLarge() = delete;
	BoostCheckpointLarge(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName);
};

//BoostCheckpointSmall
class BoostCheckpointSmall : public LocationCheckpoint
{
public:
    BoostCheckpointSmall() = delete;
	BoostCheckpointSmall(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName);
};
