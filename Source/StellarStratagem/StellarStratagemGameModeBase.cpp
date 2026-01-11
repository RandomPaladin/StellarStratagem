#include "StellarStratagemGameModeBase.h"
#include "Gameplay/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Player/StellarPlayerController.h"

void AStellarStratagemGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	UE_LOG(LogTemp, Warning, TEXT("LOG OUT"))

	const AStellarPlayerController* ExitingPlayer = Cast<AStellarPlayerController>(Exiting);
	if(!ExitingPlayer)
		return;

	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	if(!GameManager)
		return;

	GameManager->RemovePlayer(ExitingPlayer, ExitingPlayer->GetPlayerData());
}
