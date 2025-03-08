// Fill out your copyright notice in the Description page of Project Settings.


#include "Traversal/BlockBase.h"
#include "Components/SplineComponent.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"

ABlockBase::ABlockBase() {
	PrimaryActorTick.bCanEverTick = false;

}

void ABlockBase::BeginPlay() {
	Super::BeginPlay();
}

USplineComponent* ABlockBase::FindLedgeClosestToActor(const FVector ActorLocation) {
	if (Ledges.IsEmpty()) return nullptr;

	float ClosestDistance = 0.f;
	int32 ClosestIndex = 0;
	for (int i = 0; i < Ledges.Num(); i++)
	{
		const float CurrentDistance = UKismetMathLibrary::Vector_Distance(
			(Ledges[i]->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) +
				Ledges[i]->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) * 10.f), ActorLocation);

		if (CurrentDistance < ClosestDistance || i == 0)
		{
			ClosestDistance = CurrentDistance;
			ClosestIndex = i;
		}
	}
	return Ledges[ClosestIndex];
}

void ABlockBase::GetLedgeTransform(FVector HitLocation, FVector ActorLocation, ATPSCharacterBase* Character)
{
	const USplineComponent* NewSpline = FindLedgeClosestToActor(ActorLocation);
	if (!NewSpline)
	{
		Character->TempTraversalParams.HasFrontLedge = false;
		return;
	}

	if (NewSpline->GetSplineLength() >= MinLedgeWidth)
	{
		FTransform NewTransform = NewSpline->GetTransformAtDistanceAlongSpline(UKismetMathLibrary::
			FClamp(NewSpline->GetDistanceAlongSplineAtLocation(NewSpline->FindLocationClosestToWorldLocation
			(HitLocation, ESplineCoordinateSpace::Local), ESplineCoordinateSpace::Local), MinLedgeWidth / 2.f, NewSpline->GetSplineLength() -
				MinLedgeWidth / 2.f), ESplineCoordinateSpace::World);

		Character->TempTraversalParams.HasFrontLedge = true;
		Character->TempTraversalParams.FrontLedgeLocation = NewTransform.GetLocation();
		Character->TempTraversalParams.FrontLedgeNormal = UKismetMathLibrary::GetUpVector(NewTransform.Rotator());


		const USplineComponent* OppositeLedge = *OppositeLedges.Find(NewSpline);
		if (!OppositeLedge)
		{
			Character->TempTraversalParams.HasBackLedge = false;
			return;
		}

		FTransform NewTransform2 = OppositeLedge->FindTransformClosestToWorldLocation(Character->TempTraversalParams.FrontLedgeLocation, ESplineCoordinateSpace::World, false);
		Character->TempTraversalParams.HasBackLedge = true;
		Character->TempTraversalParams.BackLedgeLocation = NewTransform2.GetLocation();
		Character->TempTraversalParams.BackLedgeNormal = UKismetMathLibrary::GetUpVector(NewTransform2.Rotator());

		return;
	}

	Character->TempTraversalParams.HasFrontLedge = false;
}