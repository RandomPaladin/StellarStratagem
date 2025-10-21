#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

class AGameManager;
enum EPlanetGrade : int;
class UPlanetGradeData;

UCLASS()
class STELLARSTRATAGEM_API APlanet : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	AGameManager* GameManager;

	UPROPERTY(EditAnywhere)
	UPlanetGradeData* GradesData;

	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<EPlanetGrade> Grade;
	
	UPROPERTY(VisibleAnywhere)
	int IndustrialBuildings = 0;
	UPROPERTY(VisibleAnywhere)
	int ResearchBuildings = 0;

public:
	APlanet();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	void Setup(AGameManager* Game);
};
