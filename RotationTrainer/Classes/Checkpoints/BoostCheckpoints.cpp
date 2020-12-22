#include "BoostCheckpoints.h"

//Boost Checkpoint Large Constructors
BoostCheckpointLarge::BoostCheckpointLarge(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName)
    : LocationCheckpoint(InBoostLocation, 208, 168, 6, 600, 700)
{
    //Checkpoint construction
    LocationType = ELocationType::LT_LARGE_BOOST;
    Location = InLocation;
    Code = InCode;
    Name = InName;
}

//Boost Checkpoint Small Constructors
BoostCheckpointSmall::BoostCheckpointSmall(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName)
    : LocationCheckpoint(InBoostLocation, 144, 165, 3, 300, 350)
{
    //Checkpoint construction
    LocationType = ELocationType::LT_LARGE_BOOST;
    Location = InLocation;
    Code = InCode;
    Name = InName;
}
