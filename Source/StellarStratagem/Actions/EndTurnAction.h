#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "EndTurnAction.generated.h"

UCLASS()
class STELLARSTRATAGEM_API UEndTurnAction : public UActionBase
{
	GENERATED_BODY()

	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player) override;
};
