#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "SetAttackLineAction.generated.h"

UCLASS()
class STELLARSTRATAGEM_API USetAttackLineAction : public UActionBase
{
	GENERATED_BODY()

	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player) override;
};
