#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerPawn::TestInput()
{
	UE_LOG(LogTemp, Warning, TEXT("Input from player %s"), *GetName())
}
