#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlanetGradeData.generated.h"

UENUM(BlueprintType)
enum EPlanetGrade
{
	GradeNone = 0,
	Grade1 = 1,
	Grade2 = 2,
	Grade3 = 3,
	Grade4 = 4,
	Grade5 = 5,
	Grade6 = 6,
	Grade7 = 7,
	Grade8 = 8,
	Grade9 = 9,
	Grade10 = 10,
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
	FRuntimeFloatCurve BuildingSlotsDistribution;
};

UCLASS(Blueprintable)
class STELLARSTRATAGEM_API UPlanetGradeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EPlanetGrade>, FGradeData> Grades;
	
	int GenerateRandomBuildingSlotAmount(EPlanetGrade Grade);
	
	UFUNCTION(CallInEditor)
	void TestBuildingSlotDistribution();
};
