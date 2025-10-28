#pragma once

#include "CoreMinimal.h"
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

public:
	AShipAttackLine();

	void SetTargetLoc(const FVector& Loc) const;
	void SetTargetPlanet(const APlanet* Planet);
};
