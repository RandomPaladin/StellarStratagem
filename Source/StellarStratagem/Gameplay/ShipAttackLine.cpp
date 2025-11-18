#include "ShipAttackLine.h"
#include "Planet.h"

AShipAttackLine::AShipAttackLine()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShipAttackLine::SetTargetLoc(const FVector& Loc) const
{
	OnShipAttackLineTargetLocUpdated.Broadcast(Loc);
}

void AShipAttackLine::SetupAttackLine(AStellarPlayerController* InPlayer, APlanet* InFromPlanet, APlanet* InTargetPlanet)
{
	Player = InPlayer;
	FromPlanet = InFromPlanet;
	TargetPlanet = InTargetPlanet;
	SetTargetLoc(TargetPlanet->GetActorLocation());
}