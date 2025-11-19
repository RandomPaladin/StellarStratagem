#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipAttackLineManager.generated.h"

class APlanet;
class AShipAttackLine;
class AStellarPlayerController;
class AGameManager;

UCLASS()
class STELLARSTRATAGEM_API AShipAttackLineManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	AStellarPlayerController* LocalPlayer;
	UPROPERTY()
	AGameManager* GameManager;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AShipAttackLine> ShipAttackLineTemplate;
	UPROPERTY(VisibleAnywhere)
	TArray<AShipAttackLine*> ShipAttackLines;

	UPROPERTY(VisibleAnywhere)
	AShipAttackLine* CurrentShipAttackLine;

	UFUNCTION()
	void SetupShipAttackLineManager();
	UFUNCTION()
	void OnIncomingAttackLinesUpdated();

public:
	AShipAttackLineManager();
	virtual void BeginPlay() override;

	AShipAttackLine* CreateAttackLine();
	void RemoveAttackLine(AShipAttackLine* AttackLine);
	UFUNCTION(BlueprintCallable)
	void CancelShipAttackLine(AShipAttackLine* AttackLine);

	AShipAttackLine* GetShipAttackLineToPlanet(const APlanet* TargetPlanet) const;
};
