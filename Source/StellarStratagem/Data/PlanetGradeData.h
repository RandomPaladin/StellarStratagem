#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlanetGradeData.generated.h"

UENUM(BlueprintType)
enum EPlanetGrade
{
	Grade1,
	Grade2,
	Grade3,
	Grade4,
	Grade5,
	Grade6,
	Grade7,
	Grade8,
	Grade9,
	Grade10,
};

USTRUCT(BlueprintType)
struct FGradeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float GoldPerFactoryPerRound;
	UPROPERTY(EditAnywhere)
	float ShipsPerFactoryPerRound;

	UPROPERTY(EditAnywhere)
	FIntPoint BuildingSlotsRange = FIntPoint{1, 12};
	UPROPERTY(EditAnywhere)
	FFloatCurve BuildingSlotsDistribution;
};

UCLASS(Blueprintable)
class STELLARSTRATAGEM_API UPlanetGradeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EPlanetGrade>, FGradeData> Grades;
};
