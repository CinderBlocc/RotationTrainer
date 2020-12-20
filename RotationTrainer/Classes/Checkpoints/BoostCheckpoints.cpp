#include "BoostCheckpoints.h"
#include "RenderingTools.h"

//DRAW
void BoostCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
    constexpr float RadsPerSec = 2 * CONST_PI_F;

	//DRAW CIRCLE
    //Create circle
	RT::Circle IndicatorCircle(BoostLocation, Quat(), CircleRadius);
	IndicatorCircle.location.Z = 10; // Set floor height
	
    //Calculate distance percentage for dynamic line thickness and circle steps
	float DistancePercent = RT::GetVisualDistance(InCanvas, InFrustum, InCamera, IndicatorCircle.location);
	IndicatorCircle.lineThickness = 10 * DistancePercent;

    //Clamping distance percent clamps the number of steps to max
    constexpr int MinCircleSteps = 8;
	constexpr int MaxCircleSteps = 80;
    int CalculatedSteps = static_cast<int>(MaxCircleSteps * min(DistancePercent, 1));
	IndicatorCircle.steps = max(CalculatedSteps, MinCircleSteps);

    //Draw original circle
    RT::Matrix3 CircleMatrix(IndicatorCircle.orientation);
	IndicatorCircle.orientation = RT::AngleAxisRotation((-RadsPerSec / 8) * InSeconds, CircleMatrix.up);
    IndicatorCircle.DrawSegmented(InCanvas, InFrustum, 8, .5f);

    //Draw secondary circle spinning the opposite direction
	if(LocationType == ELocationType::LT_LARGE_BOOST)
	{
		IndicatorCircle.radius *= .67f;
		IndicatorCircle.location.Z += 25;
		IndicatorCircle.orientation = RT::AngleAxisRotation((RadsPerSec / 8) * InSeconds, CircleMatrix.up);

		IndicatorCircle.DrawSegmented(InCanvas, InFrustum, 8, .5f);
	}


	//DRAW CONE
    //Create cone
    RT::Cone IndicatorCone(BoostLocation, Vector(0,0,-1));
	IndicatorCone.radius = 10 * ConeSize;
	IndicatorCone.height = 50 * ConeSize;

    //Animate cone vertical movement
	float ConeMovementRange = ConeMaxHeight - ConeMinHeight;
	float InterpHeight = ConeMinHeight + ConeMovementRange * sinf(InSeconds * 3.f);
	IndicatorCone.location.Z = InterpHeight;

	//Animate circle and cone spinning
	IndicatorCone.rollAmount = (RadsPerSec / 5) * InSeconds;
	if(InFrustum.IsInFrustum(IndicatorCone.location, IndicatorCone.height / 2))
	{
		IndicatorCone.thickness = 1.5f;
		IndicatorCone.Draw(InCanvas);
	}
}

//CHECK COLLISION
bool BoostCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
	RT::Cylinder CollisionCylinder = RT::Cylinder(BoostLocation, CircleRadius, CylinderHeight);
	//return cylinder.IsInCylinder(car.GetLocation());
    RT::Line CarLine(InCarLocations.LastLocation, InCarLocations.CurrentLocation);
    return CollisionCylinder.LineCrossesCylinder(CarLine);
}


BoostCheckpoint::BoostCheckpoint(Vector InBoostLocation, float InCircleRadius, float InCylinderHeight, float InConeSize, float InConeMinHeight, float InConeMaxHeight)
    : BoostLocation(InBoostLocation), CircleRadius(InCircleRadius), CylinderHeight(InCylinderHeight), ConeSize(InConeSize), ConeMinHeight(InConeMinHeight), ConeMaxHeight(InConeMaxHeight) {}


//Boost Checkpoint Large Constructors
BoostCheckpointLarge::BoostCheckpointLarge(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName)
    : BoostCheckpoint(InBoostLocation, 208, 168, 6, 600, 700)
{
    //Checkpoint construction
    LocationType = ELocationType::LT_LARGE_BOOST;
    Location = InLocation;
    Code = InCode;
    Name = InName;
}


//Boost Checkpoint Small Constructors
BoostCheckpointSmall::BoostCheckpointSmall(Vector InBoostLocation, std::string InCode, ELocation InLocation, std::string InName)
    : BoostCheckpoint(InBoostLocation, 144, 165, 3, 300, 350)
{
    //Checkpoint construction
    LocationType = ELocationType::LT_LARGE_BOOST;
    Location = InLocation;
    Code = InCode;
    Name = InName;
}
