#pragma once
#include "Checkpoint.h"

namespace RT
{
    class Frustum;
}

//BoostCheckpoint base class
class LocationCheckpoint : public Checkpoint
{
public:
    LocationCheckpoint() = delete;
    LocationCheckpoint(Vector InLocation, float InRadius, int InBoostSetAmount = -1);
    LocationCheckpoint(Vector InLocation, float InCircleRadius, float InCylinderHeight, float InConeSize, float InConeMinHeight, float InConeMaxHeight, int InBoostSetAmount = -1);

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

protected:
    Vector LocationVec;
	float CircleRadius;
	float CylinderHeight;
	float ConeSize;
	float ConeMinHeight;
	float ConeMaxHeight;
    int BoostSetAmount;
};
