#include "ServerManager.h"
#include "GameManager.h"

AServerManager::AServerManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AServerManager::BeginPlay()
{
	Super::BeginPlay();
}

void AServerManager::TryCreateGame(AStellarPlayerController* Player, const FString& GameCode)
{
	//Ensure creation is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO CREATE GAME OUTSIDE OF SERVER"))
		return;
	}

	//Ensure game with this code doesn't already exist
	if(Games.Contains(GameCode))
	{
		UE_LOG(LogTemp, Error, TEXT("GAME WITH GIVEN CODE ALREADY EXISTS"))
		return;
	}
	
	//Spawn new game
	AGameManager* SpawnedGame = GetWorld()->SpawnActor<AGameManager>(GameManagerTemplate);
	Games.Add(GameCode, SpawnedGame);
	
	UE_LOG(LogTemp, Warning, TEXT("CREATING NEW GAME %s"), *GameCode)
	
	//Add player to game
	Games[GameCode]->AddPlayer(Player);
}

void AServerManager::TryJoinGame(AStellarPlayerController* Player, const FString& GameCode)
{
	//Ensure joining is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO JOIN GAME OUTSIDE OF SERVER"))
		return;
	}

	//Ensure game with this code exists
	if(!Games.Contains(GameCode))
	{
		UE_LOG(LogTemp, Error, TEXT("GAME WITH GIVEN CODE DOESN'T EXIST"))
		return;
	}
	
	//Ensure player isn't already connected to game
	if(Games[GameCode]->GetConnectedPlayers().Contains(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("TRYING TO CONNECT PLAYER TO GAME THEY'RE ALREADY CONNECTED TO"))
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("JOINING EXISTING GAME %s"), *GameCode)
	
	//Add player to game
	Games[GameCode]->AddPlayer(Player);
}

void AServerManager::TryLeaveGame(AStellarPlayerController* Player)
{
	//Ensure joining is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO LEAVE GAME OUTSIDE OF SERVER"))
		return;
	}

	//Ensure player is actually in a non started game
	FString JoinedGameCode;
	AGameManager* JoinedGame = nullptr;
	for (TTuple<FString, AGameManager*> Game : Games)
	{
		//Ignore running games
		if(Game.Value->GetGameStarted())
			continue;

		//Find game containing given player
		TArray<AStellarPlayerController*> Players = Game.Value->GetConnectedPlayers();
		if(Players.Contains(Player))
		{
			JoinedGameCode = Game.Key;
			JoinedGame = Game.Value;
			break;
		}
	}

	if(!JoinedGame)
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO LEAVE GAME WHILE NOT CURRENTLY IN A GAME"))
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("LEAVING EXISTING GAME %s"), *JoinedGameCode)
	
	//Remove player from game
	Games[JoinedGameCode]->RemovePlayer(Player);

	//If all players leave game, destroy game
	if(Games[JoinedGameCode]->GetConnectedPlayers().Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME %s HAS NO CONNECTED PLAYERS, DESTROYING GAME"), *JoinedGameCode)
		Games[JoinedGameCode]->Destroy();
		Games.Remove(JoinedGameCode);
	}
}

void AServerManager::OnGameSpawnComplete(AGameManager* Game) const
{
	OnGameCreatedOrJoined.Broadcast(Game);
}
