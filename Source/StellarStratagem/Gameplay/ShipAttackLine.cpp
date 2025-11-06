#include "ShipAttackLine.h"
#include "Planet.h"

AShipAttackLine::AShipAttackLine()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComp = CreateDefaultSubobject<USceneComponent>("RootSceneComp");
	RootComponent = RootComp;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("AttackLineMesh");
	StaticMeshComp->SetupAttachment(RootComp);
}

void AShipAttackLine::SetTargetLoc(const FVector& Loc) const
{
	const FVector StartToTargetVec = Loc - GetActorLocation();
	float Dist = StartToTargetVec.Size();

	StaticMeshComp->SetRelativeRotation(StartToTargetVec.ToOrientationRotator());
	StaticMeshComp->SetRelativeScale3D({Dist, 1.f, 1.f});
}

void AShipAttackLine::SetupAttackLine(AStellarPlayerController* InPlayer, APlanet* InFromPlanet, APlanet* InTargetPlanet)
{
	Player = InPlayer;
	FromPlanet = InFromPlanet;
	TargetPlanet = InTargetPlanet;
	SetTargetLoc(TargetPlanet->GetActorLocation());
}