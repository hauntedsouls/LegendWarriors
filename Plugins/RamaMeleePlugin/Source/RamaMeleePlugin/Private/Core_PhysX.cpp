// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaMeleePluginPrivatePCH.h"
#include "RamaMeleeCore.h"


//Apex issues
#if PLATFORM_ANDROID || PLATFORM_HTML5 || PLATFORM_IOS
#ifdef WITH_APEX
#undef WITH_APEX
#endif
#define WITH_APEX 0
#endif //APEX EXCLUSIONS
 
//Rama PX Include
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"

#define P2UV(param) ( P2UVector(param) )
#define U2PV(param) ( U2PVector(param) )

#define P2UR(PxQuatParam) ( P2UQuat(PxQuatParam).Rotator() )
#define P2UQ(PxQuatParam) ( P2UQuat(PxQuatParam) )
//Rama PX Include

//~~~~

//Draws with default shape color if not specified to draw red
// allows easy visualization of specific PxShapes of the component!
static void DrawBody_Internal(
	UWorld* World, 
	PxShape* Shape, 
	const FTransform& ShapeGlobalTrans, 
	const FQuat& ShapeGlobalRot,
	float Thickness,
	bool DrawRed
){
	//Sphere
	if(Shape->getGeometryType() == PxGeometryType::eSPHERE)
	{
		PxSphereGeometry SpherePxGeom;
		Shape->getSphereGeometry(SpherePxGeom);
		
		DrawDebugSphere(
			World, 
			ShapeGlobalTrans.GetLocation(), 
			SpherePxGeom.radius,// * BodyMinScaleAbs,  //Dont need to scale the GEOM!
			24, 
			DrawRed ? FColor::Red : FColor::Yellow, 
			false, 
			-1.f //LifeTime 
		);
		return;
		//~~~~
	}
	 
	//Box
	if(Shape->getGeometryType() == PxGeometryType::eBOX)
	{
		PxBoxGeometry BoxPxGeom;	
		Shape->getBoxGeometry(BoxPxGeom);
		 
		URamaMeleeCore::DrawBox(
			World, 
			ShapeGlobalTrans.GetLocation(), 
			P2UVector(BoxPxGeom.halfExtents), 	//Box Extents
			ShapeGlobalRot, 
			DrawRed ? FColor::Red : FColor::Yellow, 
			Thickness	//Thickness!!!
		); 
		return;
		//~~~~
	}
	
	//Capsule
	if(Shape->getGeometryType() == PxGeometryType::eCAPSULE)
	{
		PxCapsuleGeometry CapsulePxGeom;	
		Shape->getCapsuleGeometry(CapsulePxGeom);
		
		//Axis of Capsule 
		//URamaMeleeCore::DrawLine(ShapeGlobalTrans.GetLocation(), ShapeGlobalTrans.GetLocation() + ShapeGlobalRot.Rotator().Vector() * 256 );
		   
		URamaMeleeCore::DrawCapsule(
			true, 	//PhysX mode so the FQuat rotation is drawn correctly for PhysX! 
			World, 
			ShapeGlobalTrans.GetLocation(), 
			CapsulePxGeom.halfHeight, 
			CapsulePxGeom.radius, 
			ShapeGlobalRot, 
			DrawRed ? FColor::Red : FColor::Yellow, 
			false,  			//persistent
			-1.f, 				//Lifetime
			0, 				//depth
			Thickness/2
		); 
		return;
		//~~~~
	}
	
} 
static void DrawBody(UWorld* World, FBodyInstance* Body,float Thickness,TArray<int32>* PxShapesToDrawRed)
{
	if(!World || !Body) return;
	//~~~~~~~~~~~~~~~~~~~~~~~~
	 
#if PHYSICS_INTERFACE_PHYSX

	//Always Assuming Sync Scene
	FPhysScene* PhysScene = World->GetPhysicsScene();
	PxScene* PScene = PhysScene->GetPxScene();
	{
		SCOPED_SCENE_READ_LOCK(PScene);
		PxRigidActor* PRigidActor = FPhysicsInterface::GetPxRigidActor_AssumesLocked(Body->GetPhysicsActorHandle());
		if(!PRigidActor) return;
		//~~~~~~~~~~~~~~~~~

		//Get Shapes
		TArray<PxShape*, TInlineAllocator<8>> PShapes;
		PShapes.AddZeroed(PRigidActor->getNbShapes());
		PRigidActor->getShapes(PShapes.GetData(), PShapes.Num());

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		//PhysX Global Transform from Body Global Transform
		PxTransform BodyTransP = U2PTransform(Body->GetUnrealWorldTransform());
		
		//Global Rotation
		const PxQuat& BodyGlobalRot = BodyTransP.q;

		// Iterate over each shape
		for(int32 ShapeIdx=0; ShapeIdx<PShapes.Num(); ShapeIdx++)
		{
			PxShape* Shape = PShapes[ShapeIdx];
			check(Shape);

			//Shape Local Transform
			PxTransform PLocalShape = Shape->getLocalPose();
			
			//Shape Global Transform
			PxTransform ShapeGlobalTransform = BodyTransP.transform(PLocalShape);
			
			//Shape Global Rot
			PxQuat ShapeGlobalRot = BodyGlobalRot * PLocalShape.q;
			
			//Shape Global U Transform
			FTransform ShapeGlobalTransU = P2UTransform(ShapeGlobalTransform);
			
			//Internal
			bool DrawRed = false;
			if(PxShapesToDrawRed)
			{
				DrawRed = PxShapesToDrawRed->Contains(ShapeIdx);
			} 
			DrawBody_Internal(World,Shape,ShapeGlobalTransU, P2UQuat(ShapeGlobalRot),Thickness,DrawRed);
		} 
	}
#else
	UE_LOG(RamaMeleePlugin, Warning, TEXT("Compiled without support for PhysX Interface!!!!!!"));
#endif
}

void URamaMeleeCore::DrawSkeletalComp(USkeletalMeshComponent* SkelComp,float Thickness,FRamaMeleeDamageMap* PxShapesToDrawRed)
{
	if(!SkelComp) return;
	//~~~~~~~~~~~~~~~
	
	//Should be valid
	check(SkelComp->GetWorld());
	 
	//Part of Skeletal Mesh Component
	TArray<FBodyInstance*>& Bodies = SkelComp->Bodies;
   
	//For each body
	for(int32 v = 0; v < SkelComp->Bodies.Num(); v++)
	{
		FBodyInstance* EachBody = SkelComp->Bodies[v];
		
		if(!EachBody) continue;
		if(!EachBody->IsValidBodyInstance()) continue;
		//~~~~~~~~~~~~~~~~
		  
		TArray<int32>* ShapeIndicies = nullptr;
		if(PxShapesToDrawRed)
		{
			FRamaMeleeDamageInfo* FoundInfo = PxShapesToDrawRed->GetDamageInfoForBodyIndex(v);
			if(FoundInfo)
			{ 
				ShapeIndicies = &FoundInfo->ShapeIndicies;
			}
		}
	
		DrawBody(SkelComp->GetWorld(),EachBody, Thickness,ShapeIndicies);
	}
}

void URamaMeleeCore::DrawPrimitiveComp(UPrimitiveComponent* PrimComp,float Thickness,FRamaMeleeDamageMap* PxShapesToDrawRed)
{
	if(!PrimComp) return;
	//~~~~~~~~~~~~~~~
	 
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(PrimComp);
	if(SkelComp)
	{
		DrawSkeletalComp(SkelComp,Thickness,PxShapesToDrawRed);
		return;
		//~~~~
	}
	 
	//Should be valid
	check(PrimComp->GetWorld());
	
	TArray<int32>* ShapeIndicies = nullptr;
	if(PxShapesToDrawRed)
	{
		FRamaMeleeDamageInfo* FoundInfo = PxShapesToDrawRed->GetDamageInfoForBodyIndex(0);
		if(FoundInfo)
		{  
			ShapeIndicies = &FoundInfo->ShapeIndicies;
		}
	} 
	//If not Skeletal, assume only 1 Body
	DrawBody(PrimComp->GetWorld(), PrimComp->GetBodyInstance(),Thickness,ShapeIndicies);
}



void URamaMeleeCore::GetMeleeSweepData(
	USkeletalMeshComponent* SkelComp, 
	const TArray<FTransform>& StartBodyPose,
	TArray<FMeleeSweepData>& DamageShapes,
	FRamaMeleeDamageMap& DamageMap
){
	if(!SkelComp) return;
	if(!SkelComp->GetWorld()) return;
	
	//Part of Skeletal Mesh Component
	TArray<FBodyInstance*>& Bodies = SkelComp->Bodies;
   
	//Valid?
	if(Bodies.Num() < 1)
	{
		return;
	}
	
	//Always assuming Sync Scene
	FPhysScene* PhysScene = SkelComp->GetWorld()->GetPhysicsScene();
	PxScene* PScene = PhysScene->GetPxScene();
	{
		SCOPED_SCENE_READ_LOCK(PScene);
			
		//Get the Root Body
		for(int32 v = 0; v < Bodies.Num(); v++)
		{ 
			FBodyInstance* EachBody = Bodies[v];
		
			//Valid?
			if(!EachBody->IsValidBodyInstance())
			{
				continue;
			}
			
			//Default is non-damaging
			TArray<int32>* ShapeIndicies = nullptr;
			FRamaMeleeDamageInfo* FoundInfo = DamageMap.GetDamageInfoForBodyIndex(v);
			if(!FoundInfo)
			{
				//Not a Damaging Body
				continue;
			}
			 
			ShapeIndicies = &FoundInfo->ShapeIndicies;
			
			//Root Body's PxActor
			PxRigidActor* PRigidActor = FPhysicsInterface::GetPxRigidActor_AssumesLocked(EachBody->GetPhysicsActorHandle());
			if(!PRigidActor) return;
			//~~~~~~~~~~~~~~~~~

			//Get Shapes
			TArray<PxShape*, TInlineAllocator<8>> PShapes;
			
			PShapes.AddZeroed(PRigidActor->getNbShapes());
			PRigidActor->getShapes(PShapes.GetData(), PShapes.Num());

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			//! Carry over the Rotation from the Root Body Transform!
			PxTransform StartPxGlobal 	= U2PTransform(StartBodyPose[v]);
			PxTransform EndPxGlobal 		= U2PTransform(EachBody->GetUnrealWorldTransform());
			
			// Iterate over each shape
			for(int32 ShapeIdx=0; ShapeIdx<PShapes.Num(); ShapeIdx++)
			{
				if(ShapeIndicies)
				{
					if(!ShapeIndicies->Contains(ShapeIdx))
					{ 
						//Only sweep damage shapes!
						continue;
					}
				}
				 
				PxShape* Shape = PShapes[ShapeIdx];
				check(Shape);
		
				//Shape Local Transform
				PxTransform PLocalShape = Shape->getLocalPose();
				
				// calculate the test global pose of the actor
				PxTransform ShapeGlobalStart 	= StartPxGlobal.transform(PLocalShape);
				PxTransform ShapeGlobalEnd 		= EndPxGlobal.transform(PLocalShape);
				
				//New Data
				DamageShapes.Add(FMeleeSweepData());
			
				//~~~ Set the Start and End Info ~~~
				
				//New Shape Data
				FMeleeSweepData& ShapeData = DamageShapes.Last();
				
				//Index
				ShapeData.ShapeIndex = ShapeIdx;
				 
				//Global Shape Rotation
				ShapeData.Rotation = P2UQ(ShapeGlobalStart.q);
				
				//Start
				ShapeData.Start = P2UV(ShapeGlobalStart.p);
				
				//End
				ShapeData.End = P2UV(ShapeGlobalEnd.p);

				//Sphere
				if(Shape->getGeometryType() == PxGeometryType::eSPHERE)
				{
					PxSphereGeometry SpherePxGeom;
					Shape->getSphereGeometry(SpherePxGeom);
					ShapeData.SetSphere(SpherePxGeom.radius);
					continue;
					//~~~~~~
				}
				
				//Box
				if(Shape->getGeometryType() == PxGeometryType::eBOX)
				{
					PxBoxGeometry BoxPxGeom;	
					Shape->getBoxGeometry(BoxPxGeom);
					ShapeData.SetBox(P2UV(BoxPxGeom.halfExtents));
					continue;
					//~~~~~~
				}
				
				//Capsule
				if(Shape->getGeometryType() == PxGeometryType::eCAPSULE)
				{
					PxCapsuleGeometry CapsulePxGeom;	
					Shape->getCapsuleGeometry(CapsulePxGeom);
					ShapeData.SetCapsule(CapsulePxGeom.radius,CapsulePxGeom.halfHeight);
					continue;
					//~~~~~~
				} 
				  
				//! shape not found!
				//! cant display log message without crashing so might as well just crash
				UE_LOG(RamaMeleePlugin,Fatal,TEXT("URamaMeleeCore::GetMeleeSweepData shape type not found, must be convex hull. I need to add support for convex hulls. -Rama   For %s"), *SkelComp->GetName());
			}
		}
	}
}