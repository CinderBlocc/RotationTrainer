#pragma once
#include "Checkpoint.h"

namespace RT
{
    class Frustum;
}

class BackboardCheckpoint : public Checkpoint
{
public:
	BackboardCheckpoint() = delete;
	BackboardCheckpoint(Vector InGoalCorner, Vector InFarCorner, EAnimationDirection InAnimDir, std::string InCode, ELocation InLocation, std::string InName);

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

private:
    Vector GoalCorner;
	Vector FarCorner;
	EAnimationDirection AnimationDirection;
};
