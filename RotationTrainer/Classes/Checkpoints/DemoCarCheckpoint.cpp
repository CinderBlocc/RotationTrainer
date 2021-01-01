#include "DemoCarCheckpoint.h"
#include "RenderingTools.h"

DemoCarCheckpoint::DemoCarCheckpoint(std::string InName, Vector InLocation, Rotator InRotation)
    : LocationCheckpoint(InName, InLocation, 180), Rotation(InRotation)
{
    LocationType = ELocationType::LT_DEMO_CAR;
    bDrawCircle = false;
}

void DemoCarCheckpoint::SpawnCar(ServerWrapper Server)
{
    if(Server.IsNull())
    {
        return;
    }

    static const std::string BaseCarName = DEMOCAR_BASE_NAME;
    std::string FinalCarName = BaseCarName;

    //Get list of car names currently on the field
    std::vector<std::string> ExistingCarNames;
    ArrayWrapper<CarWrapper> Cars = Server.GetCars();
    for(int i = 0; i < Cars.Count(); ++i)
    {
        ExistingCarNames.push_back(GetNameFromCar(Cars.Get(i)));
    }

    //Get the first unique car name available.
    //The +5 is for some additional tests in case a unique name can't be found within the number of existing cars
    for(int i = 0; i < Cars.Count() + 5; ++i)
    {
        //"Demo Car #"
        std::string TestName = BaseCarName + " " + std::to_string(i + 1);
        bool bIsUniqueName = true;

        //Check if the test name matches any already existing names
        for(const auto& ExistingCarName : ExistingCarNames)
        {
            if(TestName == ExistingCarName)
            {
                bIsUniqueName = false;
                break;
            }
        }

        //Found a unique name
        if(bIsUniqueName)
        {
            FinalCarName = TestName;
            break;
        }
    }

    //23 is Octane
    Server.SpawnBot(23, FinalCarName);
    SpawnedCarName = FinalCarName;

    bIsCheckpointRevealed = true;
}

void DemoCarCheckpoint::DemolishCar()
{
    ServerWrapper Server = GetCurrentGameState();
    if(Server.IsNull()) { return; }

    ArrayWrapper<CarWrapper> Cars = Server.GetCars();
    for(int i = 0; i < Cars.Count(); ++i)
    {
        CarWrapper Car = Cars.Get(i);
        if(GetNameFromCar(Car) == SpawnedCarName)
        {
            Car.Demolish2(gameWrapperGlobal->GetLocalCar());
            return;
        }
    }
}

void DemoCarCheckpoint::SetSpawnedCarTransform(ServerWrapper Server)
{
    if(Server.IsNull())
    {
        return;
    }

    ArrayWrapper<CarWrapper> Cars = Server.GetCars();
    for(int i = 0; i < Cars.Count(); ++i)
    {
        CarWrapper Car = Cars.Get(i);
        if(GetNameFromCar(Car) == SpawnedCarName)
        {
            Car.SetLocation(LocationVec);
            Car.SetCarRotation(Rotation);
            Car.SetVelocity(Vector(0,0,0));
            Car.SetAngularVelocity(Vector(0,0,0), false);
            return;
        }
    }
}

std::string DemoCarCheckpoint::GetNameFromCar(CarWrapper Car)
{
    std::string Output = "NULL";

    if(Car.IsNull()) { return Output; }
    PriWrapper PRI = Car.GetPRI();
    if(PRI.IsNull()) { return Output; }
    UnrealStringWrapper PlayerName = PRI.GetPlayerName();
    if(!PlayerName.IsNull())
    {
        Output = PlayerName.ToString();
    }

    return Output;
}

void DemoCarCheckpoint::ResetCheckpoint()
{
    DemolishCar();
    bIsCheckpointRevealed = false;
    SpawnedCarName = "";
}

void DemoCarCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
    LocationCheckpoint::Draw(InCanvas, InCamera, InFrustum, InSeconds);
}

bool DemoCarCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
    if(LocationCheckpoint::CheckCollision(InCar, InCarLocations))
    {
        DemolishCar();
        ResetCheckpoint();
        return true;
    }

    return false;
}

ServerWrapper DemoCarCheckpoint::GetCurrentGameState()
{
    if(gameWrapperGlobal->IsInReplay())
        return gameWrapperGlobal->GetGameEventAsReplay().memory_address;
    else if(gameWrapperGlobal->IsInOnlineGame())
        return gameWrapperGlobal->GetOnlineGame();
    else
        return gameWrapperGlobal->GetGameEventAsServer();
}
