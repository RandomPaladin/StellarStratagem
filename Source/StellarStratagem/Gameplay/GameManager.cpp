#include "GameManager.h"
#include "ServerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

AGameManager::AGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGameManager::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if(ConnectedPlayers.Num() == 0)
		return true;
		
	return ConnectedPlayers.Contains(RealViewer);
}

void AGameManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGameManager, AllPlayers);
	DOREPLIFETIME(AGameManager, GameStarted);
}

void AGameManager::BeginPlay()
{
	Super::BeginPlay();

	//Get server manager
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));

	//Complete spawn
	ServerManager->OnGameSpawnComplete(this);
	OnPlayersUpdated.Broadcast();
}

void AGameManager::AddPlayer(AStellarPlayerController* Player)
{
	//Ensure adding player is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD PLAYER TO GAME OUTSIDE OF SERVER"))
		return;
	}
	
	//Add player
	AllPlayers.AddUnique(FPlayerData{Player->GetUsername()});
	AActor* PlayerActor = Player;
	ConnectedPlayers.Add(PlayerActor, Player);

	ForceNetUpdate();
	
	UE_LOG(LogTemp, Warning, TEXT("ADDED PLAYER %s TO GAME"), *Player->GetUsername());
}

void AGameManager::RemovePlayer(AStellarPlayerController* Player)
{
	//Ensure adding player is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD PLAYER TO GAME OUTSIDE OF SERVER"))
		return;
	}
	
	//Remove player
	AllPlayers.RemoveAll([Player](const FPlayerData& PlayerData) {return PlayerData.Username == Player->GetUsername();});
	AActor* PlayerActor = Player;
	ConnectedPlayers.Remove(PlayerActor);

	ForceNetUpdate();

	UE_LOG(LogTemp, Warning, TEXT("REMOVED PLAYER %s FROM GAME"), *Player->GetUsername());
}

void AGameManager::StartGame()
{
	//Ensure game start is done on server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("TRYING TO START GAME OUTSIDE OF SERVER"));
		return;
	}

	//Ensure game isn't already started
	if(GameStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME IS ALREADY STARTED"));
		return;
	}

	//Start game
	GameStarted = true;
}

void AGameManager::OnRep_GameStarted() const
{
	OnGameStateUpdated.Broadcast(GameStarted);
}

void AGameManager::OnRep_ConnectedPlayers() const
{
	OnPlayersUpdated.Broadcast();
}

TArray<AStellarPlayerController*> AGameManager::GetConnectedPlayers() const
{
	TArray<AStellarPlayerController*> PlayerControllers;
	ConnectedPlayers.GenerateValueArray(PlayerControllers);

	return PlayerControllers;
}
