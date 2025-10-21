#include "PlanetManager.h"
#include "GameManager.h"
#include "Planet.h"
#include "ServerManager.h"
#include "Kismet/GameplayStatics.h"

APlanetManager::APlanetManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APlanetManager::BeginPlay()
{
	Super::BeginPlay();

	AServerManager* ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));
	ServerManager->OnGameCreatedOrJoined.AddDynamic(this, &APlanetManager::OnGameCreatedOrJoined);
}

void APlanetManager::OnGameCreatedOrJoined(AGameManager* Game)
{
	//Only run on server
	if(!HasAuthority())
		return;

	//Create planets
	const int PlanetAmount = Game->GetPlayerAmount() * SpawnPlanetsPerPlayer;
	for (int i = 0; i < PlanetAmount; i++)
	{
		FVector SpawnLoc = {FMath::RandRange(SpawnPlanetXLocRange.X, SpawnPlanetXLocRange.Y), FMath::RandRange(SpawnPlanetYLocRange.X, SpawnPlanetYLocRange.Y), 0.f};
		FRotator SpawnRot = {0.f, FMath::RandRange(SpawnPlanetRotRange.X, SpawnPlanetRotRange.Y), 0.f};
		APlanet* SpawnedPlanet = GetWorld()->SpawnActor<APlanet>(PlanetTemplate, SpawnLoc, SpawnRot);
		SpawnedPlanet->Setup(Game);
		Planets.Add(SpawnedPlanet);
	}
}