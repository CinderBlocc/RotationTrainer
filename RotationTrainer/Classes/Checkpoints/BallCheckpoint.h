#pragma once
#include "Checkpoint.h"

namespace RT
{
    class Frustum;
}

class BallCheckpoint : public Checkpoint
{
public:
    BallCheckpoint() = delete;
	BallCheckpoint(std::string InName);

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

private:
    float Radius;
};
