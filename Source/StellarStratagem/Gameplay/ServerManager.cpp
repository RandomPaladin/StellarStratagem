#include "ServerManager.h"
#include "GameManager.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

AServerManager::AServerManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AServerManager::BeginPlay()
{
	Super::BeginPlay();
}

void AServerManager::TryConnectToGame(AStellarPlayerController* Player, const FString& GameCode)
{
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("TRYING TO CONNECT A USER TO A GAME OUTSIDE OF THE SERVER"));
		return;
	}
	
	if(Games.Contains(GameCode))
	{
		if(!Games[GameCode]->GetPlayers().Contains(Player))
		{
			UE_LOG(LogTemp, Warning, TEXT("ADDING PLAYER %s TO EXISTING GAME %s"), *Player->GetUsername(), *GameCode);
			Games[GameCode]->AddPlayer(Player);
			//TODO SEND MESSAGE TO CLIENT THAT THEY ARE NOT PART OF THIS GAME IF GAME IS ALREADY STARTED
			return;
		}
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ADDING PLAYER %s TO NEW GAME %s"), *Player->GetUsername(), *GameCode);
		//TODO SEND MESSAGE TO CLIENT THAT GAME DOES NOT EXIST
		AGameManager* SpawnedGame = GetWorld()->SpawnActor<AGameManager>(GameManagerTemplate);
		SpawnedGame->SetupGame();
		SpawnedGame->AddPlayer(Player);
		Games.Add(GameCode, SpawnedGame);
	}
}
