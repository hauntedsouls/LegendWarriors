// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaMeleePluginPrivatePCH.h"
#include "RamaMeleeShape.h"
#include "RamaMeleeCore.h"


void FMeleeSweepData::Draw(UWorld* World, const FVector& Loc, float Thickness, const FColor& Color,float LifeTime) const
{ 
	//Sphere
	if(IsSphere())
	{
		DrawDebugSphere(
			World, 
			Loc, 
			GetSphereRadius(),		//WorldCollision.h
			24, 								//segments
			Color, 
			false,		//not persist 
			-1.f, 			//LifeTime 
			0				//depth
		);
		return;
	}
	//Box
	if(IsBox())
	{
		URamaMeleeCore::DrawBox(
			World, 
			Loc, 
			GetBox(),  //WorldCollision.h (because this is a FCollisionShape)
			Rotation,
			Color, 
			Thickness,
			false, 		//not persist
			LifeTime,
			0	 			//depth
		);
		return;
	}
	
	//Capsule
	if(IsCapsule())
	{
		URamaMeleeCore::DrawCapsule(
			true,									//PhysX Mode 
			World, 
			Loc, 
			GetCapsuleHalfHeight(),  //WorldCollision.h   (because this is a FCollisionShape)
			GetCapsuleRadius(), 
			Rotation,  
			Color, 
			false, 			//not persist
			LifeTime, 
			0, 				//Depth priority 
			Thickness
		);
	
		return;
	}
}