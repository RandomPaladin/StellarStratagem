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

void AGameManager::BeginPlay()
{
	Super::BeginPlay();

	//Get server manager
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));

	//Complete spawn
	ServerManager->OnGameSpawnComplete(this);
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
	AActor* PlayerActor = Player;
	ConnectedPlayers.Add(PlayerActor, Player);
	AllPlayers.Add(FPlayerData{Player->GetUsername()});
	UE_LOG(LogTemp, Warning, TEXT("ADDED PLAYER ON SERVER"));
}

void AGameManager::OnRep_ConnectedPlayers() const
{
	OnPlayersUpdated.Broadcast();
}

void AGameManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGameManager, AllPlayers);
}

TArray<AStellarPlayerController*> AGameManager::GetPlayers() const
{
	TArray<AStellarPlayerController*> PlayerControllers;
	ConnectedPlayers.GenerateValueArray(PlayerControllers);

	return PlayerControllers;
}
