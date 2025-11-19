#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum EActionType
{
	ActionType_None,
	ActionType_Build,
	ActionType_EndTurn,
	ActionType_ProductionDistributionUpdate,
	ActionType_SetAttackLine,
	ActionType_CancelAttackLine,
};