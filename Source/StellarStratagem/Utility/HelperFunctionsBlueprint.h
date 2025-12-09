#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StellarStratagem/Player/PlayerData.h"
#include "HelperFunctionsBlueprint.generated.h"

UCLASS()
class STELLARSTRATAGEM_API UHelperFunctionsBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, meta=(DisplayName="Equal (PlayerData)"), Category="PlayerData")
	static bool Equal_PlayerData(const FPlayerData& A, const FPlayerData& B);

	UFUNCTION(BlueprintPure, meta=(DisplayName="NotEqual (PlayerData)"), Category="PlayerData")
	static bool NotEqual_PlayerData(const FPlayerData& A, const FPlayerData& B);

	UFUNCTION(BlueprintPure, meta=(DisplayName="IsValid (PlayerData)"), Category="PlayerData")
	static bool IsValid_PlayerData(const FPlayerData& A);
};
