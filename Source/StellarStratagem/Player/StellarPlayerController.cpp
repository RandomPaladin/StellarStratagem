#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Gameplay/ServerManager.h"

void AStellarPlayerController::BeginPlay()
{
	//Get server manager
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));

	//Set random username
	Username = FString::FromInt(FMath::RandRange(0, 10000000));
	
	Super::BeginPlay();
}

void AStellarPlayerController::SendAction_Server_Implementation(const FActionData& ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("RECEIVED DATA %d"), ActionData.NumberTest);
}

void AStellarPlayerController::TryConnectToGame_Server_Implementation(const FString& GameCode)
{
	ServerManager->TryConnectToGame(this, GameCode);
}