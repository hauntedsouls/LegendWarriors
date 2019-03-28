// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaMeleePluginPrivatePCH.h"
#include "RamaMeleeWeapon.h"
 
//////////////////////////////////////////////////////////////////////////
// URamaMeleeWeapon

URamaMeleeWeapon::URamaMeleeWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ 
	//!Plugin version
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	//Type
	//		You most likely will want to use a custom object channel for this via
	//			Project Settings -> Collision
	BodyInstance.SetObjectType(ECC_WorldDynamic);
	
	//Ignore all by default, using PhysX sweeps. 
	//		If want melee weapons to block each other set this component to have an appropriate Object Type
	//			and add that object to MeleeTraceObjectTypes below;
	BodyInstance.SetResponseToAllChannels(ECR_Ignore);
	BodyInstance.SetResponseToChannel(ECC_Visibility,ECR_Block);
	   
	MeleeTraceObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
}      
 
void URamaMeleeWeapon::InitializeComponent()
{
	Super::InitializeComponent();
	//~~~~~~~~~~~~~~~~~~~~~
	
	UpdateSwingPoseBodies();
}
void URamaMeleeWeapon::UpdateSwingPoseBodies()
{
	SwingPrevPose.Empty();
	SwingPrevPose.AddZeroed(Bodies.Num());
}

void URamaMeleeWeapon::SetSkeletalMesh(class USkeletalMesh* NewMesh, bool bReinitPose)
{
	Super::SetSkeletalMesh(NewMesh,bReinitPose);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	UpdateSwingPoseBodies();
}

void URamaMeleeWeapon::Draw()
{
	if(!IsValid()) return;
	//~~~~~~~~~~~~~
	 
	if(bHiddenInGame) return;
	//~~~~~~~~~~~~~~~~~~~
	 
	//Dedicated Server
	if(GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer)
	{
		return;
	}
	 
	//Draw the Melee Comp! Handles Skeletal Physics Assets!!!!!! (my own code)
	URamaMeleeCore::DrawPrimitiveComp(this,DrawShapes_Thickness,&DamageMap);
}
	
bool URamaMeleeWeapon::MeleeSweep(FHitResult& Hit, const TArray<FTransform>& BodyPreviousPose)
{
	AActor* Actor = GetOwner();
	FBodyInstance* RootBody = GetBodyInstance();
	if(!RootBody) return false;
	
	//RamaMeleeShape.h for definition of FMeleeSweepData
	TArray<FMeleeSweepData> DamageShapesData;
	
	URamaMeleeCore::GetMeleeSweepData(
		this, 
		BodyPreviousPose, 
		DamageShapesData,
		DamageMap
	);

	//if(WITH_EDITOR) UE_LOG(LogTemp,Warning,TEXT("dmg shape count %d"), DamageShapesData.Num());
	   
	
	//Do All Damage Shapes First!
	
	bool AnyHitOccurred = false;
	for(FMeleeSweepData& EachMeleeSweep : DamageShapesData ) //Damage
	{
		if(DrawSweeps)
		{
			EachMeleeSweep.DrawStart(
				GetWorld(), 
				DrawShapes_Thickness, 
				FColor::Red, 
				DrawShapes_Duration
			); 
			EachMeleeSweep.DrawEnd(
				GetWorld(), 
				DrawShapes_Thickness, 
				FColor::Red, 
				DrawShapes_Duration
			);
		}
		  
		//Sweep as Damage
		if(PerformDeepTrace)
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(Actor);
			int32 Iterations = 0;
			int32 MaxIterations = 10000;
			while(Iterations < MaxIterations)
			{  
				if(Iterations >= MaxIterations - 1)
				{ 
					UE_LOG(LogTemp,Warning,TEXT("Melee Weapon Infinite Loop Occurred, Tell Rama!"));
					return true;
				}
				
				if(URamaMeleeCore::MeleeSweep(
					GetWorld(),
					ActorsToIgnore,
					Hit,
					EachMeleeSweep,
					FCollisionObjectQueryParams(MeleeTraceObjectTypes)
				)){
					//BroadCast
					if(RamaMeleeWeapon_OnHit.IsBound())
					{
						RamaMeleeWeapon_OnHit.Broadcast(Hit.GetActor(), Hit.GetComponent(), Hit.ImpactPoint, Hit.ImpactNormal, EachMeleeSweep.ShapeIndex, Hit.BoneName, Hit);
					}
					  
					//Perform the trace again, ignoring the hit actor, to find ALL _unique_ actors along same path.
					ActorsToIgnore.Add(Hit.GetActor());
					Iterations++;
					AnyHitOccurred = true;
					continue;
				}
				 
				//No hit occurred this time, done!
				 
				//Performance
				if(DeepTrace_SingleBodyMode)
				{
					return AnyHitOccurred;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if(URamaMeleeCore::MeleeSweep(
				GetWorld(),
				Actor,
				Hit,
				EachMeleeSweep,
				FCollisionObjectQueryParams(MeleeTraceObjectTypes)
			)){
				//BroadCast
				if(RamaMeleeWeapon_OnHit.IsBound())
				{
					RamaMeleeWeapon_OnHit.Broadcast(Hit.GetActor(), Hit.GetComponent(), Hit.ImpactPoint, Hit.ImpactNormal, EachMeleeSweep.ShapeIndex, Hit.BoneName, Hit);
				}
				  
				return true;
				//~~~~ Save performance, we already hit something
			}
		}
	}
	    
	return false;
}
 
void URamaMeleeWeapon::StartSwingDamage()
{
	if(!SwingPrevPose.Num()) 
	{
		return;
	}
	
	//Clear previous data
	SwingPrevPose[0].SetLocation(FVector::ZeroVector);
	
	DoingSwingTraces = true;
}
void URamaMeleeWeapon::StopSwingDamage()
{
	DoingSwingTraces = false;
}
	
void URamaMeleeWeapon::SwingTick()
{ 
	if(!SwingPrevPose.Num())
	{
		return;
	}
	
	if(!SwingPrevPose[0].GetLocation().IsZero()) 
	{
		FHitResult Hit;
		MeleeSweep(Hit, SwingPrevPose);
		//Hit info is broadcasted inside of MeleeSweep above.
		
		//Draw Swing Lines?
		if(DrawLines)
		{
			for(int32 v = 0; v < Bodies.Num(); v++)
			{
				if(!DamageMap.ContainsBodyIndex(v))
				{
					continue;
				}
				
				FBodyInstance* EachBody = Bodies[v];
				if(!EachBody)
				{
					return;
				}
				
				if(!SwingPrevPose.IsValidIndex(v))
				{
					continue;
				}  
				 
				//if(WITH_EDITOR) UE_LOG(LogTemp,Warning,TEXT("Draw Lines index %d %s"), v, *EachBody->GetUnrealWorldTransform().GetLocation().ToString());
				
				URamaMeleeCore::DrawLine(
					GetWorld(),
					EachBody->GetUnrealWorldTransform().GetLocation(),
					SwingPrevPose[v].GetLocation(),
					DrawShapes_Thickness,
					FColor::Red,
					DrawShapes_Duration
				);
			}
		}
	}
	
	//Store Previous Position!  
	for(int32 v = 0; v < Bodies.Num(); v++)
	{
		FBodyInstance* EachBody = Bodies[v];
		 
		if(!EachBody || !EachBody->IsValidBodyInstance() || !SwingPrevPose.IsValidIndex(v))
		{
			continue;
		}  
		SwingPrevPose[v] = EachBody->GetUnrealWorldTransform();
	}
	
}