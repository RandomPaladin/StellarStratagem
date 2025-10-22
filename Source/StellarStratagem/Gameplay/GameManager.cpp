#include "GameManager.h"
#include "Planet.h"
#include "ServerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

#pragma region Setup / Lobby

AGameManager::AGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
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
	
	DOREPLIFETIME(AGameManager, Round);
	DOREPLIFETIME(AGameManager, GameStarted);
	DOREPLIFETIME(AGameManager, AllPlayers);
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
	
	UE_LOG(LogTemp, Warning, TEXT("ADDED PLAYER %s TO GAME"), *Player->GetUsername())
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

	UE_LOG(LogTemp, Warning, TEXT("REMOVED PLAYER %s FROM GAME"), *Player->GetUsername())
}

#pragma endregion

#pragma region Gameplay

void AGameManager::StartGame()
{
	//Ensure game start is done on server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO START GAME OUTSIDE OF SERVER"))
		return;
	}

	//Ensure game isn't already started
	if(GameStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("GAME IS ALREADY STARTED"))
		return;
	}

	//Start game
	AwaitedPlayers = AllPlayers;
	GameStarted = true;

	//Create planets
	const int PlanetAmount = GetPlayerAmount() * SpawnPlanetsPerPlayer;
	for (int i = 0; i < PlanetAmount; i++)
	{
		FVector SpawnLoc = {FMath::RandRange(SpawnPlanetXLocRange.X, SpawnPlanetXLocRange.Y), FMath::RandRange(SpawnPlanetYLocRange.X, SpawnPlanetYLocRange.Y), 0.f};
		FRotator SpawnRot = {0.f, FMath::RandRange(SpawnPlanetRotRange.X, SpawnPlanetRotRange.Y), 0.f};
		APlanet* SpawnedPlanet = GetWorld()->SpawnActor<APlanet>(PlanetTemplate, SpawnLoc, SpawnRot);
		SpawnedPlanet->Setup(this);
		Planets.Add(SpawnedPlanet);
	}
	
	//Grant a starting planet to each player
	const UEnum* GradeEnum = StaticEnum<EPlanetGrade>();
	const int GradeEnumMiddleIndex = (GradeEnum->NumEnums() - 1) / 2;
	for (FPlayerData Player : AllPlayers)
	{
		//Find a suitable planet to grant to player (one closest to the middle grade)
		int StartPlanetIndex = -1;
		int BestDist = 10000;
		for(int i = 0; i < Planets.Num(); i++)
		{
			//Ignore already owned planets
			if(Planets[i]->IsOwnedByPlayer())
				continue;

			//Check if this planet's grade is closer to the middle grade
			const int GradeEnumIndex = GradeEnum->GetIndexByValue(Planets[i]->GetGrade());
			const int NewDist = FMath::Abs(GradeEnumMiddleIndex - GradeEnumIndex);
			if(NewDist < BestDist)
			{
				BestDist = NewDist;
				StartPlanetIndex = i;
			}
		}

		//Ensure a starting planet was found
		if(StartPlanetIndex == -1)
		{
			UE_LOG(LogTemp, Error, TEXT("COULDN'T FIND SUITABLE START PLANET FOR PLAYER"))
			continue;
		}
		
		//Grant starting planet to player
		Planets[StartPlanetIndex]->SetOwningPlayer(Player);
	}
}

void AGameManager::EndTurn(AStellarPlayerController* Player)
{
	//Ensure this is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO END TURN OUTSIDE OF SERVER"))
		return;
	}
	
	//Ensure player is awaited
	if(!AwaitedPlayers.ContainsByPredicate([Player](const FPlayerData& PlayerData){ return PlayerData.Username == Player->GetUsername(); }))
	{
		UE_LOG(LogTemp, Error, TEXT("PLAYER IS NOT AWAITED"))
		return;
	}

	//Remove awaited player from list
	UE_LOG(LogTemp, Warning, TEXT("PLAYER %s ENDED THEIR TURN"), *Player->GetUsername())
	AwaitedPlayers.RemoveAll([Player](const FPlayerData& PlayerData){ return PlayerData.Username == Player->GetUsername(); });

	//Move on to the next round if all awaited players took their turn
	if(AwaitedPlayers.Num() == 0)
		GoToNextRound();
}

void AGameManager::GoToNextRound()
{
	//Increment round
	UE_LOG(LogTemp, Warning, TEXT("ALL PLAYERS ENDED THEIR TURN, GOING TO NEXT ROUND"))
	Round++;
	AwaitedPlayers = AllPlayers;

	//Generate building resources
	//TODO
	
	//Build planned buildings

	//Complete ship movement

	//Resolve combat
}

void AGameManager::RegisterPlanet(APlanet* Planet)
{
	if(HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("REGISTERING PLANET SHOULD ONLY OCCUR ON CLIENT"))
		return;
	}
	
	Planets.AddUnique(Planet);
}

#pragma endregion

#pragma region Replication

void AGameManager::OnRep_GameStarted() const
{
	OnGameStateUpdated.Broadcast(GameStarted);
}

void AGameManager::OnRep_ConnectedPlayers() const
{
	OnPlayersUpdated.Broadcast();
}

#pragma endregion

TArray<AStellarPlayerController*> AGameManager::GetConnectedPlayerControllers() const
{
	TArray<AStellarPlayerController*> PlayerControllers;
	ConnectedPlayers.GenerateValueArray(PlayerControllers);

	return PlayerControllers;
}
