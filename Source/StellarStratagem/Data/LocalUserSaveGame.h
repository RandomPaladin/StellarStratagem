#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LocalUserSaveGame.generated.h"

UCLASS()
class STELLARSTRATAGEM_API ULocalUserSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Username;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CurrentGameCode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsCurrentGameCreator;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> GameCodes;
};
