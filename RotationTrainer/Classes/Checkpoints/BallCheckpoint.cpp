#include "BallCheckpoint.h"
#include "RenderingTools.h"

//CONSTRUCTOR
BallCheckpoint::BallCheckpoint(std::string InName)
    : Checkpoint(ELocationType::LT_BALL, ELocation::LOC_BALL, 0, InName, InName), Radius(92.75) {}

//DRAW
void BallCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
	RT::Cone IndicatorCone;
	IndicatorCone.radius = 30;
	IndicatorCone.height = 100;
	IndicatorCone.direction = Vector(0,0,-1);
	IndicatorCone.location = Vector(0, 0, IndicatorCone.height + (Radius * 2) + 50);

	RT::Circle IndicatorCircle;
	IndicatorCircle.steps = 16;
	IndicatorCircle.radius = Radius * 2;
	IndicatorCircle.lineThickness = 2;
	IndicatorCircle.location = Vector(0,0,-40);

	//canvas.SetColor(222,222,222,255);
	if(InFrustum.IsInFrustum(IndicatorCone.location, IndicatorCone.height / 2))
	{
		IndicatorCone.thickness = 1.5f;
		IndicatorCone.Draw(InCanvas);
	}

	//canvas.SetColor(222,222,222,150);
	IndicatorCircle.Draw(InCanvas, InFrustum);

	for(int i = 0; i < 15; ++i)
	{
		IndicatorCircle.radius *= .9f;
		IndicatorCircle.location.Z -= 10;
		IndicatorCircle.Draw(InCanvas, InFrustum);
	}
}

//CHECK COLLISION
bool BallCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
    //No point in checking ball checkpoint's collision. Ball checkpoint is completed when a goal is scored
    return false;
}
