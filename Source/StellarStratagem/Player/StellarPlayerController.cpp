#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Gameplay/ServerManager.h"

void AStellarPlayerController::BeginPlay()
{
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));
	
	Super::BeginPlay();
}

void AStellarPlayerController::SendAction_Implementation(const FActionData& ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("RECEIVED DATA %d"), ActionData.NumberTest);
}

void AStellarPlayerController::TryConnectToGame_Implementation(const FString& GameCode)
{
	ServerManager->TryConnectToGame(this, GameCode);
}