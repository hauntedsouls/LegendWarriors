// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#pragma once

//Core
#include "RamaMeleeCore.h"
 
#include "Components/SkeletalMeshComponent.h"
#include "RamaMeleeWeapon.generated.h"
  
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams( FRamaMeleeHitSignature, class AActor*, HitActor, class UPrimitiveComponent*, HitComponent, const FVector&, ImpactPoint, const FVector&, ImpactNormal, int32, ShapeIndex, FName, HitBoneName, const struct FHitResult&, HitResult );
 
UCLASS(ClassGroup=Rama, BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class RAMAMELEEPLUGIN_API URamaMeleeWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	URamaMeleeWeapon(const FObjectInitializer& ObjectInitializer); 
	
	UPROPERTY(EditAnywhere, Category="Rama Melee Weapon")
	TArray<TEnumAsByte<EObjectTypeQuery> > MeleeTraceObjectTypes;
	 
	/**If HitWasDamage returns false, then the weapon hit another object with a non-damaging shape. Returns false if no hit at all. */
	UPROPERTY(BlueprintAssignable, Category="Rama Melee Weapon")
	FRamaMeleeHitSignature RamaMeleeWeapon_OnHit;
	
	/** Draw the PhysX Shapes, indicating which ones do damage! <3 Rama */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawShapes = true;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawSweeps = true;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawLines = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	float DrawShapes_Thickness = 3;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	float DrawShapes_Duration = 2;
	
	/**
		If you enable this feature, more traces will be performed, so that within a single trace, even if a hit occurs, I will perform the trace again in that same tick, ignoring the hit actor, and finding any other actors that are within the same trace path.
		
		This resolves an issue for fast swings where multiple objects get hit in the same trace, but only 1 hit is reported, when you have a weapon that is supposed to pass through each hit object rather than stopping on the first hit.
		
		The physX engine stops on the first hit, so I must create another trace to keep finding other objects along the same single trace path.
		
		Potential disadvantage of this feature is that more traces will be performed, but it will only happen for the number of hits that occur, the number of unique objects along the trace path.
		
		Please note this feature does _not_ result in multiple hit reports on the same object, within the same trace, because I ignore each hit object and continue trace looking for other unique objects.
		
		If your melee weapon design expects to recoil or stop on the first valid hit, you will not need this feature.
		
		This is a per-weapon feature in case you have some weapons that DO cut through all possible objects, and other weapons that stop on the first hit object.
		
		Enjoy!
		
		♥
		
		Rama
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	bool PerformDeepTrace = false;
	
	/** If this is true, even if a weapon has multiple damaging bodies, only the first damaging body will be swept against all possible actors along the swing path, for performance reasons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	bool DeepTrace_SingleBodyMode = false;
	
	/** Each Body can have many shapes. BodyIndex specifies which body you want to provide shape indicies for. Each shape whose index is supplied here will be treated as a damaging part of the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	FRamaMeleeDamageMap DamageMap;
	
//~~~~~~~~~~~~~
// Core Functions
//~~~~~~~~~~~~~
public:
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void StartSwingDamage();
	
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void StopSwingDamage();
	
	/** Reset and rebuild Swing Pose List from current number of bodies in skeletal mesh asset */
	UFUNCTION(Category = "Rama Melee Weapon", BlueprintCallable)
    void UpdateSwingPoseBodies();
	
//~~~~~~~~~~~~~
// Core Utility
//~~~~~~~~~~~~~
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Rama Melee Weapon")
	bool DoingSwingTraces = false;
	 
	/** Returns false if no hit at all. */
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	bool MeleeSweep(FHitResult& Hit, const TArray<FTransform>& BodyPreviousPose);
	
	TArray<FTransform> SwingPrevPose;
	void SwingTick();
	
public:
	void Draw();
	 
	FORCEINLINE bool IsValid() const
	{
		return SkeletalMesh != nullptr;
	}
	
	virtual void InitializeComponent() override;
	
	virtual void SetSkeletalMesh(class USkeletalMesh* NewMesh, bool bReinitPose = true) override;
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Tick
	//		By using tick to draw, it shows up in Editor Preview! Woohoo!
	//			-Rama
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override
	{
		Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		if(DoingSwingTraces)
		{
			SwingTick();
		}
		 
		//Draw
		if(DrawShapes)
		{  
			Draw();
		}
	}
	
	/*
	FString ECRToString(ECollisionResponse ECR) const
	{
		switch (ECR)
		{
			case ECR_Block : return "Block";
			case ECR_Overlap : return "Overlap";
			case ECR_Ignore : return "Ignore";
		}
		return "None";
	}
	*/
	
};



