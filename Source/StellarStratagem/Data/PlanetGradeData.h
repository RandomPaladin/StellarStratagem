#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StellarStratagem/Actions/BuildingType.h"
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
	int GoldPerFactoryPerRound;
	UPROPERTY(EditAnywhere)
	float ShipsPerFactoryPerRound;

	UPROPERTY(EditAnywhere)
	FIntPoint BuildingSlotsRange = FIntPoint{1, 12};
	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve BuildingSlotsDistribution;
};

USTRUCT(BlueprintType)
struct FBuildingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int GoldCost;
};

UCLASS(Blueprintable)
class STELLARSTRATAGEM_API UPlanetGradeData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EPlanetGrade>, FGradeData> Grades;
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EBuildingType>, FBuildingData> Buildings;
	UPROPERTY(EditAnywhere)
	float TechXPPercentPerResearchBuildingPerRound = 0.05f;
	UPROPERTY(EditAnywhere)
	int SpawnPlanetsPerPlayer = 5;
	UPROPERTY(EditAnywhere)
	FIntPoint DistanceBetweenPlanetsRange = FIntPoint{5, 15};
	UPROPERTY(EditAnywhere)
	float DistanceBetweenPlanetsToUnrealUnitsMultiplier = 350.f;
	
	int GenerateRandomBuildingSlotAmount(EPlanetGrade Grade);
	
	UFUNCTION(CallInEditor)
	void TestBuildingSlotDistribution();
};
