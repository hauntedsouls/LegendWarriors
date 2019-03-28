// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#pragma once

#if WITH_EDITOR
	#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#endif

#include "RamaMeleeShape.h"
#include "RamaMeleeCore.generated.h"

USTRUCT(BlueprintType)
struct FRamaMeleeDamageInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="Rama Melee")
	int32 BodyIndex = 0;
	
	UPROPERTY(EditAnywhere, Category="Rama Melee")
	TArray<int32> ShapeIndicies;

	FRamaMeleeDamageInfo()
	{
		ShapeIndicies.Add(0);
	}
}; 
USTRUCT(BlueprintType)
struct FRamaMeleeDamageMap
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="Rama Melee")
	TArray<FRamaMeleeDamageInfo> Bodies;

	bool ContainsBodyIndex(int32 InBodyIndex) const
	{
		for( const FRamaMeleeDamageInfo& EachBody : Bodies)
		{
			if(EachBody.BodyIndex == InBodyIndex)
			{
				return true;
			}
		}
		return false;
	} 
	
	FRamaMeleeDamageInfo* GetDamageInfoForBodyIndex(int32 InBodyIndex)
	{
		for( FRamaMeleeDamageInfo& EachBody : Bodies)
		{
			if(EachBody.BodyIndex == InBodyIndex)
			{
				return &EachBody;
			}
		}
		return nullptr;
	} 
	FRamaMeleeDamageMap(){}
}; 


UCLASS()
class URamaMeleeCore : public UObject
{
	GENERATED_BODY()
public:
	//~~~~~~~~~~~~~~~~
	//		Drawing
	//~~~~~~~~~~~~~~~~
	
	// Draw Line
	static FORCEINLINE void DrawLine 
	(
	const UWorld* InWorld, 
		const FVector& Start,
		const FVector& End,
		const float& Size = 7,
		const FColor& Color = FColor::Red,
		const float Duration=-1.f
	)  {

#if WITH_EDITOR
		DrawDebugLine(
			InWorld, 
			Start, 
			End, 
			Color, 
			false,
			Duration,
			0, //depth
			Size //thickness
		);
#endif
	}
	
	//Draw Box With Thickness
	static void DrawBox(
		const UWorld* InWorld, 
		FVector const& Center, 
		FVector const& Box, 
		const FQuat& Rotation,
		FColor const& Color, 
		float Thickness,
		bool bPersistentLines=false,
		float LifeTime=-1,
		uint8 DepthPriority=0	
	);
	
	//Draw Capsule that has PhysX-Friendly option to work with PxTransforms properly
	static void DrawCapsule(
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
	);

	//~~~~~~~~~~~~~~~~
	//	Joy Shape Sweeps
	//~~~~~~~~~~~~~~~~
	static FORCEINLINE bool MeleeSweep(
		UWorld* World,
		AActor* ActorToIgnore,
		FHitResult& Hit,
		const FVector& Start,
		const FVector& End,
		const FRamaMeleeShape& RamaMeleeShape,
		const FCollisionObjectQueryParams& ObjectQueryParams,
		const FQuat& Rot = FQuat::Identity
	){
		if(!World) return false;
		
		//Expose (not till multi threading solid tho)
		FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Sphere Sweep")), true, ActorToIgnore);
		TraceParams.bTraceComplex = true;
		TraceParams.AddIgnoredActor(ActorToIgnore);
		
		//Physical Material
		TraceParams.bReturnPhysicalMaterial = true;
		  
		return World->SweepSingleByObjectType(
			Hit, 
			Start, 
			End, 
			Rot, 
			ObjectQueryParams, 
			RamaMeleeShape, 
			TraceParams
		);
	}
	 
	static FORCEINLINE bool MeleeSweep(
		UWorld* World,
		TArray<AActor*> ActorsToIgnore,
		FHitResult& Hit,
		const FVector& Start,
		const FVector& End,
		const FRamaMeleeShape& RamaMeleeShape,
		const FCollisionObjectQueryParams& ObjectQueryParams,
		const FQuat& Rot = FQuat::Identity
	){
		if(!World) return false;
		
		//Expose (not till multi threading solid tho)
		FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Sphere Sweep")), true); //complex
		TraceParams.bTraceComplex = true;
		
		for(AActor* Each : ActorsToIgnore)
		{
			TraceParams.AddIgnoredActor(Each);
		}
		
		//Physical Material
		TraceParams.bReturnPhysicalMaterial = true;
		  
		return World->SweepSingleByObjectType(
			Hit, 
			Start, 
			End, 
			Rot, 
			ObjectQueryParams, 
			RamaMeleeShape, 
			TraceParams
		);
	}
	
	static FORCEINLINE bool MeleeSweep(
		UWorld* World,
		AActor* ActorToIgnore,
		FHitResult& Hit,
		const FMeleeSweepData& MeleeSweep,
		const FCollisionObjectQueryParams& ObjectQueryParams
	){
		return URamaMeleeCore::MeleeSweep(
			World,
			ActorToIgnore,
			Hit,
			MeleeSweep.Start,
			MeleeSweep.End,
			MeleeSweep,
			ObjectQueryParams, 
			MeleeSweep.Rotation
		);
	} 
	
	static FORCEINLINE bool MeleeSweep(
		UWorld* World,
		TArray<AActor*> ActorsToIgnore,
		FHitResult& Hit,
		const FMeleeSweepData& MeleeSweep,
		const FCollisionObjectQueryParams& ObjectQueryParams
	){
		return URamaMeleeCore::MeleeSweep(
			World,
			ActorsToIgnore,
			Hit,
			MeleeSweep.Start,
			MeleeSweep.End,
			MeleeSweep,
			ObjectQueryParams, 
			MeleeSweep.Rotation
		);
	} 
	//~~~~~~~~~~~~~~~~
	//	PhysX-Related
	//~~~~~~~~~~~~~~~~
	
	//Sweeps from StartBodyPose to current position!
	static void GetMeleeSweepData(
		USkeletalMeshComponent* SkelComp, 
		const TArray<FTransform>& StartBodyPose,
		TArray<FMeleeSweepData>& DamageShapes,
		FRamaMeleeDamageMap& DamageMap
	);
	
	//Draw
	static void DrawSkeletalComp(USkeletalMeshComponent* SkelComp, float Thickness = 7, FRamaMeleeDamageMap* PxShapesToDrawRed = nullptr);
	static void DrawPrimitiveComp(UPrimitiveComponent* PrimComp, float Thickness = 7, FRamaMeleeDamageMap* PxShapesToDrawRed = nullptr);

};