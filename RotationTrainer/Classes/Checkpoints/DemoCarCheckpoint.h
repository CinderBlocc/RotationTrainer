#pragma once
#include "LocationCheckpoint.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"

namespace RT
{
    class Frustum;
}

class DemoCarCheckpoint : public LocationCheckpoint
{
public:
    DemoCarCheckpoint() = delete;
    DemoCarCheckpoint(std::string InName, Vector InLocation, Rotator InRotation);

    //When the sequence gets to one of these checkpoints, it should check if the checkpoint is revealed
    //If it isn't revealed yet, spawn the car
    bool IsCheckpointRevealed() { return bIsCheckpointRevealed; }
    void SpawnCar(ServerWrapper Server);
    void SetSpawnedCarTransform(ServerWrapper Server);

    //Set Revealed back to false in preparation for next sequence
    void ResetCheckpoint() override;

    void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds) override;
    bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations) override;

private:
    Rotator Rotation;
    bool bIsCheckpointRevealed = false;
    std::string SpawnedCarName;
    std::string GetNameFromCar(CarWrapper Car);
};
