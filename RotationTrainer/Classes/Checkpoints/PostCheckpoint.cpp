#include "PostCheckpoint.h"
#include "RenderingTools.h"

//CONSTRUCTORS
PostCheckpoint::PostCheckpoint(Vector InNearCorner, Vector InFarCorner, EAnimationDirection InAnimDir, std::string InCode, ELocation InLocation, std::string InName)
    : Checkpoint(ELocationType::LT_POST, InLocation, 0, InCode, InName),
      NearCorner(InNearCorner), FarCorner(InFarCorner), AnimationDirection(InAnimDir) {}

//DRAW
void PostCheckpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, RT::Frustum InFrustum, float InSeconds)
{
	Vector PC = NearCorner;
	Vector FC = FarCorner;
	const float Y = PC.Y;
	constexpr float depth = -500;

    //Points that comprise the post gate and the start location of the chevrons
	Vector BottomInner      = {PC.X, Y,         PC.Z};
	Vector BottomOuter      = {FC.X, Y,         PC.Z};
	Vector TopInner         = {PC.X, Y,         FC.Z};
	Vector TopOuter         = {FC.X, Y,         FC.Z};
	Vector TopInnerDepth    = {PC.X, Y - depth, FC.Z};
	Vector BottomInnerDepth = {PC.X, Y - depth, PC.Z};

    //Create chevron
    constexpr int QuarterTurn = static_cast<int>(90 * CONST_DegToUnrRot);
    constexpr int HalfTurn = static_cast<int>(180 * CONST_DegToUnrRot);
	RT::Chevron BaseChevron = RT::Chevron(Vector(0,0,0), RotatorToQuat(Rotator(0, HalfTurn, QuarterTurn)), 150, 150, 100, 0, 0);
	Vector ChevronStart = (BottomOuter + TopOuter) * .5f;
	Vector ChevronEnd = (BottomInner + TopInner) * .5f;

    //Draw Chevrons
	if(AnimationDirection == EAnimationDirection::ANIM_CW)
	{
		BaseChevron.orientation = RotatorToQuat(Rotator(0, 0, QuarterTurn));
	}
	BaseChevron.DrawAlongLine(InCanvas, InFrustum, ChevronStart, ChevronEnd, 100, 300, InSeconds);
	
	//Draw Grid
	RT::Grid GateGrid((BottomInner + TopInnerDepth) * .5f, Quat(), 500, 1440, 5, 14);
    GateGrid.location.Z -= 10;
    LinearColor ColorBackup = InCanvas.GetColor();
    InCanvas.SetColor(LinearColor{ColorBackup.R, ColorBackup.G, ColorBackup.B, 75});
	GateGrid.Draw(InCanvas, InFrustum, false);
    InCanvas.SetColor(ColorBackup);

	//Draw outline of gate
	const float Thiccness = 10 * RT::GetVisualDistance(InCanvas, InFrustum, InCamera, PC);
	RT::Line WallVerticalLine(  TopInner,         BottomInner,      Thiccness     );
	RT::Line BottomLine(        BottomInner,      BottomInnerDepth, Thiccness     );
	RT::Line DepthVerticalLine( BottomInnerDepth, TopInnerDepth,    Thiccness * 3 );
	RT::Line TopLine(           TopInnerDepth,    TopInner,         Thiccness     );
	
    TopLine.lineBegin.Z = TopLine.lineEnd.Z = TopLine.lineBegin.Z - 10;

	WallVerticalLine.DrawWithinFrustum(  InCanvas, InFrustum );
	BottomLine.DrawWithinFrustum(        InCanvas, InFrustum );
	DepthVerticalLine.DrawWithinFrustum( InCanvas, InFrustum );	
	TopLine.DrawWithinFrustum(           InCanvas, InFrustum );
}

//CHECK COLLISION
bool PostCheckpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
	Vector PC = NearCorner;
	Vector FC = FarCorner;
	const float Y = PC.Y;
	constexpr float Depth = -500;

	Vector BottomInner      = { PC.X, Y,         PC.Z};
	Vector TopInner         = { PC.X, Y,         FC.Z};
	Vector TopInnerDepth    = { PC.X, Y - Depth, FC.Z};
	Vector BottomInnerDepth = { PC.X, Y - Depth, PC.Z};

	RT::Line CarLine(InCarLocations.LastLocation, InCarLocations.CurrentLocation);
	bool bIntersectingTri1 = RT::Triangle(BottomInner, BottomInnerDepth, TopInnerDepth).LineTriangleIntersection(CarLine);
	bool bIntersectingTri2 = RT::Triangle(TopInner,    BottomInner,      TopInnerDepth).LineTriangleIntersection(CarLine);
	if(bIntersectingTri1 || bIntersectingTri2)
	{
		//Check which direction the car was moving
		if(Location == ELocation::LOC_POST_LEFT && InCarLocations.CurrentLocation.X < InCarLocations.LastLocation.X)
		{
			return true;
		}
		if(Location == ELocation::LOC_POST_RIGHT && InCarLocations.CurrentLocation.X > InCarLocations.LastLocation.X)
		{
			return true;
		}
	}
	
	return false;
}
