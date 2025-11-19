#pragma once

#include "CoreMinimal.h"
#include "ActionType.h"
#include "BuildingType.h"
#include "StellarStratagem/Gameplay/ShipAttackLineData.h"
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
	int IntValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int IntValue2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ShipAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NumberValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EBuildingType> BuildingType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipAttackLineData ShipAttackLine;

	FActionData()
	{
		IntValue = 0;
		IntValue2 = 0;
		ShipAmount = 0;
		NumberValue = 0.f;
		ActionType = ActionType_None;
		BuildingType = BuildingType_None;
		ShipAttackLine = {};
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

	FActionResult(const bool InSucceeded, FString InMessage)
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
