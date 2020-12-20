#pragma once
#include "Checkpoint.h"

namespace RT
{
    class Frustum;
}

//BoostCheckpoint base class
class BoostCheckpoint : public Checkpoint
{
public:
    BoostCheckpoint() = delete;
    BoostCheckpoint(Vector InBoostLocation, float InCircleRadius, float InCylinderHeight, float InConeSize, float InConeMinHeight, float InConeMaxHeight);

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

protected:
    Vector BoostLocation;
	float CircleRadius;
	float CylinderHeight;
	float ConeSize;
	float ConeMinHeight;
	float ConeMaxHeight;
};

//BoostCheckpointLarge
class BoostCheckpointLarge : public BoostCheckpoint
{
public:
    BoostCheckpointLarge() = delete;
	BoostCheckpointLarge(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName);
};

//BoostCheckpointSmall
class BoostCheckpointSmall : public BoostCheckpoint
{
public:
    BoostCheckpointSmall() = delete;
	BoostCheckpointSmall(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName);
};
