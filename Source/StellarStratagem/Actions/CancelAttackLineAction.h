#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "CancelAttackLineAction.generated.h"

UCLASS()
class STELLARSTRATAGEM_API UCancelAttackLineAction : public UActionBase
{
	GENERATED_BODY()

	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player) override;
};
