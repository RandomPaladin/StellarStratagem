#pragma once

#include "CoreMinimal.h"
#include "Planet.h"
#include "GameFramework/Actor.h"
#include "ShipAttackLine.generated.h"

class APlanet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipAttackLineLocUpdatedDelegate, FVector, NewTargetLoc);

UCLASS()
class STELLARSTRATAGEM_API AShipAttackLine : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	AStellarPlayerController* Player;
	UPROPERTY(VisibleAnywhere)
	APlanet* FromPlanet;
	UPROPERTY(VisibleAnywhere)
	APlanet* TargetPlanet;

protected:
	UPROPERTY(BlueprintReadOnly)
	FVector FromLoc;
	UPROPERTY(BlueprintReadOnly)
	FVector TargetLoc;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int ShipAmount;

public:
	AShipAttackLine();

	void SetFromLoc(const FVector& Loc);
	void SetTargetLoc(const FVector& Loc);
	void SetupAttackLine(AStellarPlayerController* InPlayer, APlanet* InFromPlanet, APlanet* InTargetPlanet);
	
	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AStellarPlayerController* GetOwningPlayer() const { return Player; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetFromPlanet() const { return FromPlanet; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetTargetPlanet() const { return TargetPlanet; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetShipAmount() const { return ShipAmount; }

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnShipAttackLineLocUpdatedDelegate OnShipAttackLineFromLocUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnShipAttackLineLocUpdatedDelegate OnShipAttackLineTargetLocUpdated;
};
