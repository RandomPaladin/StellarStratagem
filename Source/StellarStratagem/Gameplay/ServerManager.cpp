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
	//Ensure connection is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO CONNECT PLAYER TO GAME OUTSIDE OF SERVER"))
		return;
	}
	
	if(Games.Contains(GameCode)) //Handle joining existing game
	{
		//Ensure player isn't already connected to game
		if(Games[GameCode]->GetPlayers().Contains(Player))
		{
			UE_LOG(LogTemp, Warning, TEXT("TRYING TO CONNECT PLAYER TO GAME THEY'RE ALREADY CONNECTED TO"))
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("ADDING PLAYER %s TO EXISTING GAME %s"), *Player->GetUsername(), *GameCode)
	}
	else //Handle making new game
	{
		//Spawn new game
		AGameManager* SpawnedGame = GetWorld()->SpawnActor<AGameManager>(GameManagerTemplate);
		Games.Add(GameCode, SpawnedGame);
		
		UE_LOG(LogTemp, Warning, TEXT("ADDING PLAYER %s TO NEW GAME %s"), *Player->GetUsername(), *GameCode)
	}
	
	//Add player to game
	Games[GameCode]->AddPlayer(Player);

	//Force net update for IsNetRelevantFor() update
	Games[GameCode]->ForceNetUpdate();
}

void AServerManager::OnGameSpawnComplete(AGameManager* Game) const
{
	OnGameCreatedOrJoined.Broadcast(Game);
}
