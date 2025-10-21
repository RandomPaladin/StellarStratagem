#pragma once

#include "CoreMinimal.h"
#include "PlanetManager.generated.h"

class APlanet;
class AGameManager;

UCLASS()
class STELLARSTRATAGEM_API APlanetManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlanet> PlanetTemplate;
	UPROPERTY(EditAnywhere)
	int SpawnPlanetsPerPlayer = 5;
	UPROPERTY(EditAnywhere)
	FVector2D SpawnPlanetXLocRange = {-3000.f, 3000.f};
	UPROPERTY(EditAnywhere)
	FVector2D SpawnPlanetYLocRange = {-3000.f, 3000.f};
	UPROPERTY(EditAnywhere)
	FVector2D SpawnPlanetRotRange = {0.f, 359.f};

	UPROPERTY(VisibleAnywhere)
	TArray<APlanet*> Planets;

	UFUNCTION()
	void OnGameCreatedOrJoined(AGameManager* Game);

public:
	APlanetManager();
	virtual void BeginPlay() override;
};
