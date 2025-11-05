#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
}