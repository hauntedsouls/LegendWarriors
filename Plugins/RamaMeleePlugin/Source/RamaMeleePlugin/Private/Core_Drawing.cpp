// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaMeleePluginPrivatePCH.h"
#include "RamaMeleeCore.h"
 
//~~~~~~~
//Drawing
//~~~~~~~
static ULineBatchComponent* GetDebugLineBatcher( const UWorld* InWorld, bool bPersistentLines, float LifeTime, bool bDepthIsForeground )
{
	return (InWorld ? (bDepthIsForeground ? InWorld->ForegroundLineBatcher : (( bPersistentLines || (LifeTime > 0.f) ) ? InWorld->PersistentLineBatcher : InWorld->LineBatcher)) : NULL);
}
 
void URamaMeleeCore::DrawBox(
	const UWorld* InWorld, 
	FVector const& Center, 
	FVector const& Box, 
	const FQuat& Rotation,
	FColor const& Color, 
	float Thickness, 
	bool bPersistentLines, 
	float LifeTime, 
	uint8 DepthPriority
){
	// no debug line drawing on dedicated server
	if (GEngine->GetNetMode(InWorld) == NM_DedicatedServer) return;
	
	// this means foreground lines can't be persistent 
	ULineBatchComponent* const LineBatcher = GetDebugLineBatcher( InWorld, bPersistentLines, LifeTime, (DepthPriority == SDPG_Foreground) );
	if(LineBatcher != NULL)
	{
		float LineLifeTime = (LifeTime > 0.f) ? LifeTime : LineBatcher->DefaultLifeTime;
	
		FTransform Transform(Rotation);
		FVector Start = Transform.TransformPosition(FVector( Box.X,  Box.Y,  Box.Z));
		FVector End = Transform.TransformPosition(FVector( Box.X, -Box.Y, Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector( Box.X, -Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X,  Box.Y, Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X,  Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector( Box.X,  Box.Y, Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector( Box.X,  Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector( Box.X, -Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector( Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X,  Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X,  Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector( Box.X,  Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector( Box.X,  Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector( Box.X,  Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector( Box.X, -Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector( Box.X, -Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);

		Start = Transform.TransformPosition(FVector(-Box.X,  Box.Y,  Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X,  Box.Y, -Box.Z));
		LineBatcher->DrawLine(Center + Start, Center + End, Color, DepthPriority, Thickness, LineLifeTime);
	}
}



//~~~~
//~~~~
//~~~~

//Expose
static void JoyDrawHalfCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
	float	AngleDelta = 2.0f * (float)PI / ((float)NumSides);
	FVector	LastVertex = Base + X * Radius;

	for(int32 SideIndex = 0; SideIndex < (NumSides/2); SideIndex++)
	{
		FVector	Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
		DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		LastVertex = Vertex;
	}	
}

//Expose
static void JoyDrawCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness)
{
	const float	AngleDelta = 2.0f * PI / NumSides;
	FVector	LastVertex = Base + X * Radius;

	for(int32 SideIndex = 0;SideIndex < NumSides;SideIndex++)
	{
		const FVector Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
		DrawDebugLine(InWorld, LastVertex, Vertex, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		LastVertex = Vertex;
	}
} 
void URamaMeleeCore::DrawCapsule(
	bool PhysXMode,
	const UWorld* InWorld, 
	FVector const& Center, 
	float HalfHeight, 
	float Radius, 
	const FQuat& Rotation, 
	FColor const& Color, 
	bool bPersistentLines, 
	float LifeTime, 
	uint8 DepthPriority, 
	float Thickness
){
	// no debug line drawing on dedicated server
	if (GEngine->GetNetMode(InWorld) != NM_DedicatedServer)
	{
		const int32 DrawCollisionSides = 16;

		FVector Origin = Center;
		FMatrix Axes = FQuatRotationTranslationMatrix(Rotation, FVector::ZeroVector);
		FVector XAxis = (PhysXMode) ? Axes.GetScaledAxis( EAxis::Z ) : Axes.GetScaledAxis( EAxis::X );
		FVector YAxis = Axes.GetScaledAxis( EAxis::Y );
		FVector ZAxis = (PhysXMode) ? Axes.GetScaledAxis( EAxis::X ) : Axes.GetScaledAxis( EAxis::Z );

		// Draw top and bottom circles
		float HalfAxis = FMath::Max<float>(HalfHeight - Radius, 1.f);
		FVector TopEnd = Origin + HalfAxis*ZAxis;
		FVector BottomEnd = Origin - HalfAxis*ZAxis;

		JoyDrawCircle(InWorld, TopEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
		JoyDrawCircle(InWorld, BottomEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

		// Draw domed caps
		JoyDrawHalfCircle(InWorld, TopEnd, YAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
		JoyDrawHalfCircle(InWorld, TopEnd, XAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

		FVector NegZAxis = -ZAxis;

		JoyDrawHalfCircle(InWorld, BottomEnd, YAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);
		JoyDrawHalfCircle(InWorld, BottomEnd, XAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, DepthPriority, Thickness);

		// Draw connected lines
		DrawDebugLine(InWorld, TopEnd + Radius*XAxis, BottomEnd + Radius*XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		DrawDebugLine(InWorld, TopEnd - Radius*XAxis, BottomEnd - Radius*XAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		DrawDebugLine(InWorld, TopEnd + Radius*YAxis, BottomEnd + Radius*YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		DrawDebugLine(InWorld, TopEnd - Radius*YAxis, BottomEnd - Radius*YAxis, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
	}
}
	