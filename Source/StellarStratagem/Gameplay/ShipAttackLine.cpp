#include "ShipAttackLine.h"
#include "Planet.h"

AShipAttackLine::AShipAttackLine()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShipAttackLine::SetFromLoc(const FVector& Loc)
{
	FromLoc = Loc;
	OnShipAttackLineFromLocUpdated.Broadcast(Loc);
}

void AShipAttackLine::SetTargetLoc(const FVector& Loc)
{
	TargetLoc = Loc;
	OnShipAttackLineTargetLocUpdated.Broadcast(Loc);
}

void AShipAttackLine::SetupAttackLine(AStellarPlayerController* InPlayer, APlanet* InFromPlanet, APlanet* InTargetPlanet, const int InShipAmount)
{
	Player = InPlayer;
	FromPlanet = InFromPlanet;
	TargetPlanet = InTargetPlanet;
	ShipAmount = InShipAmount;
	
	SetFromLoc(FromPlanet->GetActorLocation());
	SetTargetLoc(TargetPlanet->GetActorLocation());
}