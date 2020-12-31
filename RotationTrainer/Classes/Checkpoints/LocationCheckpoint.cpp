#include "LocationCheckpoint.h"
#include "RenderingTools.h"

LocationCheckpoint::LocationCheckpoint(std::string InName, Vector InLocation, float InRadius, int InBoostSetAmount)
    : Checkpoint(ELocationType::LT_CUSTOM_LOCATION, ELocation::LOC_DEFAULT, "LOCATION", InName),
    LocationVec(InLocation),
    CircleRadius(InRadius),
    BoostSetAmount(InBoostSetAmount)
{
    //Default height
    CylinderHeight = 200;

    //CALCULATE ALL OF THESE RELATIVE TO THE RADIUS
    ConeMinHeight = CircleRadius * 3;
    ConeMaxHeight = ConeMinHeight + (CircleRadius / 2);
    ConeSize = ConeMinHeight / 100;
}

LocationCheckpoint::LocationCheckpoint(Vector InLocation, float InCircleRadius, float InCylinderHeight, float InConeSize, float InConeMinHeight, float InConeMaxHeight, int InBoostSetAmount)
    : Checkpoint(ELocationType::LT_CUSTOM_LOCATION, ELocation::LOC_DEFAULT, "LOCATION", "LOCATION"),
    LocationVec(InLocation),
    CircleRadius(InCircleRadius),
    CylinderHeight(InCylinderHeight),
    ConeSize(InConeSize),
    ConeMinHeight(InConeMinHeight),
    ConeMaxHeight(InConeMaxHeight),
    BoostSetAmount(InBoostSetAmount) {}

//DRAW
void LocationCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
    constexpr float RadsPerSec = 2 * CONST_PI_F;

	//DRAW CIRCLE
    if(bDrawCircle)
    {
        //Create circle
	    RT::Circle IndicatorCircle(LocationVec, Quat(), CircleRadius);
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
    }


	//DRAW CONE
    //Create cone
    RT::Cone IndicatorCone(LocationVec, Vector(0,0,-1));
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
bool LocationCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
	RT::Cylinder CollisionCylinder = RT::Cylinder(LocationVec, CircleRadius, CylinderHeight);
    RT::Line CarLine(InCarLocations.LastLocation, InCarLocations.CurrentLocation);
    
    if(CollisionCylinder.LineCrossesCylinder(CarLine))
    {
        //Attempt to set the car's boost amount if required by checkpoint
        if(BoostSetAmount != -1)
        {
            BoostWrapper boost = InCar.GetBoostComponent();
            if(!boost.IsNull())
            {
                boost.SetCurrentBoostAmount(BoostSetAmount / 100.f);
            }
        }

        return true;
    }

    return false;
}
