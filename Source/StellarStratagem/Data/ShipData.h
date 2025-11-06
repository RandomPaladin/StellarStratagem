#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipData.generated.h"

UCLASS(BlueprintType)
class STELLARSTRATAGEM_API UShipData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int DefaultShipMoveDistance = 15;

	UPROPERTY(EditAnywhere)
	int MoveDistancePerTurnDivisor = 3; //The number the max distance is divided by to determine the move distance per turn
};
