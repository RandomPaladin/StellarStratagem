#pragma once

#include "CoreMinimal.h"
#include "ActionType.h"
#include "BuildingType.h"
#include "ActionBase.generated.h"

class AGameManager;
class AStellarPlayerController;

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EActionType> ActionType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Number;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Number2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EBuildingType> BuildingType;

	FActionData()
	{
		Number = 0;
		Number2 = 0;
		ActionType = ActionType_None;
		BuildingType = BuildingType_None;
	}
};

USTRUCT(BlueprintType)
struct FActionResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool Succeeded;
	UPROPERTY()
	FString Message;

	FActionResult()
	{
		Succeeded = false;
		Message = "";
	}

	FActionResult(bool InSucceeded, FString InMessage)
	{
		Succeeded = InSucceeded;
		Message = InMessage;
	}
};

UCLASS()
class STELLARSTRATAGEM_API UActionBase : public UObject
{
	GENERATED_BODY()

public:
	FActionData Data;
	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player);
};
