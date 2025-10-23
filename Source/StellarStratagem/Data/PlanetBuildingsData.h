#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StellarStratagem/Gameplay/Planet.h"
#include "PlanetBuildingsData.generated.h"

USTRUCT(BlueprintType)
struct FBuildingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int GoldCost;
};

UCLASS(Blueprintable)
class STELLARSTRATAGEM_API UPlanetBuildingsData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EBuildingType>, FBuildingData> Buildings;
};
