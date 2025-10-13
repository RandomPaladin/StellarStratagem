#include "GameManager.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

AGameManager::AGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGameManager::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	return Players.Contains(RealViewer);
}

void AGameManager::BeginPlay()
{
	Super::BeginPlay();
}

void AGameManager::SetupGame()
{
	Turn++;
}

void AGameManager::AddPlayer(AStellarPlayerController* Player)
{
	AActor* PlayerActor = Player->GetPawn();
	Players.Add(PlayerActor, Player);
}
