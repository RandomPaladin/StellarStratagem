#pragma once

#include "CoreMinimal.h"
#include "ActionBase.generated.h"

class AGameManager;
class APlayerPawn;

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

	UPROPERTY()
	int NumberTest;

	FActionData()
	{
		NumberTest = 0;
	}

	FActionData(int InNumberTest)
	{
		NumberTest = InNumberTest;
	}
};

UCLASS()
class STELLARSTRATAGEM_API UActionBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void PerformAction(AGameManager* Manager, APlayerPawn* Player) {};
};
