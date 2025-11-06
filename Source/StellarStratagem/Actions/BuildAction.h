#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "BuildAction.generated.h"

class AStellarPlayerController;

UCLASS()
class STELLARSTRATAGEM_API UBuildAction : public UActionBase
{
	GENERATED_BODY()

	virtual FActionResult PerformAction(AGameManager* GameManager, AStellarPlayerController* Player) override;
};
