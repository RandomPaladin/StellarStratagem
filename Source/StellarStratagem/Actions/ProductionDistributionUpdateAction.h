#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "ProductionDistributionUpdateAction.generated.h"

UCLASS()
class STELLARSTRATAGEM_API UProductionDistributionUpdateAction : public UActionBase
{
	GENERATED_BODY()

	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player) override;
};
