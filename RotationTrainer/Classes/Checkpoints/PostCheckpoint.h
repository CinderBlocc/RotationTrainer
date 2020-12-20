#pragma once
#include "Checkpoint.h"

namespace RT
{
    class Frustum;
}

class PostCheckpoint : public Checkpoint
{
public:
    PostCheckpoint() = delete;
    PostCheckpoint(Vector InNearCorner, Vector InFarCorner, EAnimationDirection InAnimDir, std::string InCode, ELocation InLocation, std::string InName);

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

private:
    Vector NearCorner;
	Vector FarCorner;
	EAnimationDirection AnimationDirection;
};
