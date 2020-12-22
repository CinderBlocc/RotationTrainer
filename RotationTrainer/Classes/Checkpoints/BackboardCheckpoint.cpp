#include "BackboardCheckpoint.h"
#include "RenderingTools.h"

//CONSTRUCTORS
BackboardCheckpoint::BackboardCheckpoint(Vector InGoalCorner, Vector InFarCorner, EAnimationDirection InAnimDir, std::string InCode, ELocation InLocation, std::string InName)
    : Checkpoint(ELocationType::LT_BACKBOARD, InLocation, InCode, InName), GoalCorner(InGoalCorner), FarCorner(InFarCorner), AnimationDirection(InAnimDir) {}

//DRAW
void BackboardCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
	Vector GC = GoalCorner;
	Vector FC = FarCorner;
	const float Y = GC.Y;

	Vector BottomInner = { GC.X, Y, GC.Z };
	Vector BottomOuter = { FC.X, Y, GC.Z };
	Vector TopInner =    { GC.X, Y, FC.Z };
	Vector TopOuter =    { FC.X, Y, FC.Z };

	constexpr float Thiccness = 3;
	RT::Line innerLine(  TopInner,    BottomInner, Thiccness );
	RT::Line bottomLine( BottomInner, BottomOuter, Thiccness );
	RT::Line outerLine(  BottomOuter, TopOuter,    Thiccness );
	RT::Line topLine(    TopOuter,    TopInner,    Thiccness );

	if(AnimationDirection == EAnimationDirection::ANIM_CW)
	{
		//Animation direction mirrors across X axis
		topLine = RT::Line(    TopInner,    TopOuter,    Thiccness );
		outerLine = RT::Line(  TopOuter,    BottomOuter, Thiccness );
		bottomLine = RT::Line( BottomOuter, BottomInner, Thiccness );
		innerLine = RT::Line(  BottomInner, TopInner,    Thiccness );
	}

	//canvas.SetColor(255,0,255,255);

	constexpr float SegmentLength = 50;
	constexpr float GapLength = 50;
	constexpr float Speed = 2;
	innerLine.DrawSegmentedAutomatic(  InCanvas, InFrustum, SegmentLength, GapLength, Speed, InSeconds );
	bottomLine.DrawSegmentedAutomatic( InCanvas, InFrustum, SegmentLength, GapLength, Speed, InSeconds );
	outerLine.DrawSegmentedAutomatic(  InCanvas, InFrustum, SegmentLength, GapLength, Speed, InSeconds );
	topLine.DrawSegmentedAutomatic(    InCanvas, InFrustum, SegmentLength, GapLength, Speed, InSeconds );
}

//CHECK COLLISION
bool BackboardCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
	if(!InCar.IsOnWall())
	{
		return false;
	}

	Vector CurrentLocation = InCar.GetLocation();

	bool bIsInsideHeight = CurrentLocation.Z < FarCorner.Z && CurrentLocation.Z > GoalCorner.Z;
	bool bIsInsideDepth = CurrentLocation.Y < FarCorner.Y && CurrentLocation.Y > GoalCorner.Y - 200;
	bool bIsInsideWidth = false;
	if(Location == ELocation::LOC_BACKBOARD_LEFT)
	{
		bIsInsideWidth = CurrentLocation.X < FarCorner.X && CurrentLocation.X > GoalCorner.X;
	}
	if(Location == ELocation::LOC_BACKBOARD_RIGHT)
	{
		bIsInsideWidth = CurrentLocation.X > FarCorner.X && CurrentLocation.X < GoalCorner.X;
	}

	if(bIsInsideWidth && bIsInsideHeight && bIsInsideDepth)
	{
		return true;
	}

	return false;
}
