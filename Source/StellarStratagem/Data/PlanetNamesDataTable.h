#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlanetNamesDataTable.generated.h"

USTRUCT(BlueprintType)
struct FPlanetNamesDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString PlanetName;
};