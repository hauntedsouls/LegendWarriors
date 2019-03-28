/*
	Joy Shape!
	
	For drawing PhysX shapes and converting from PhysX shapes and rotations straight to collision sweep data
	
	Collision shape with Rotation data and drawing capability
	
	by Rama
*/
#pragma once

//FCollisionShape currently resides in WorldCollision.h

//Joyful Collision Shape
struct FRamaMeleeShape : public FCollisionShape
{ 	
	/** Is the shape currently a box? Constructor initializes to ECollisionShape::Line. */
	bool IsBox() const
	{
		return ShapeType == ECollisionShape::Box;
	}
	
	/** Is the shape currently a sphere? Constructor initializes to ECollisionShape::Line */
	bool IsSphere() const
	{
		return ShapeType == ECollisionShape::Sphere;
	}	
	
	/** Is the shape currently a capsule? Constructor initializes to ECollisionShape::Line */
	bool IsCapsule() const
	{
		return ShapeType == ECollisionShape::Capsule;
	}
			
	//Sphere
	FRamaMeleeShape()
	{
		SetSphere(128);
	}
	FRamaMeleeShape(float Radius)
	{
		SetSphere(Radius);
	}
	
	//Box
	FRamaMeleeShape(const FVector& BoxExtents)
	{
		SetBox(BoxExtents);
	}
	
	//Capsule
	FRamaMeleeShape(float Radius, float HH)
	{
		SetCapsule(Radius,HH);
	}
}; 

//For PhysX traces
struct FMeleeSweepData : public FRamaMeleeShape
{ 
	//Shape Index
	int32 ShapeIndex;
	
	FVector Start;
	FVector End;
	FQuat Rotation;
	
	//Drawing
	void DrawStart(UWorld* World, float Thickness, const FColor& Color, float LifeTime=-1) const
	{
		Draw(World,Start,Thickness,Color,LifeTime);
	}
	void DrawEnd(UWorld* World, float Thickness, const FColor& Color,float LifeTime=-1) const
	{
		Draw(World,End,Thickness,Color,LifeTime);
	}
	void Draw(UWorld* World, const FVector& Loc, float Thickness, const FColor& Color,float LifeTime=-1)  const;
	
	FMeleeSweepData()
	{ 
		Start = End = FVector::ZeroVector;
		Rotation = FQuat::Identity;
		ShapeIndex = 0;
	}
};