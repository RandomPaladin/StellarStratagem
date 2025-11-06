#pragma once

#include "CoreMinimal.h"
#include "Planet.h"
#include "GameFramework/Actor.h"
#include "ShipAttackLine.generated.h"

class APlanet;

UCLASS()
class STELLARSTRATAGEM_API AShipAttackLine : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(VisibleAnywhere)
	AStellarPlayerController* Player;
	UPROPERTY(VisibleAnywhere)
	APlanet* FromPlanet;
	UPROPERTY(VisibleAnywhere)
	APlanet* TargetPlanet;

public:
	AShipAttackLine();

	void SetTargetLoc(const FVector& Loc) const;
	void SetupAttackLine(AStellarPlayerController* InPlayer, APlanet* InFromPlanet, APlanet* InTargetPlanet);

	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AStellarPlayerController* GetOwningPlayer() const { return Player; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetFromPlanet() const { return FromPlanet; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetTargetPlanet() const { return TargetPlanet; }
};
