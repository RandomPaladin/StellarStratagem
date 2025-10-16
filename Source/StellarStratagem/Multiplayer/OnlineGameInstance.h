#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineGameInstance.generated.h"

class IOnlineSubsystem;

UCLASS()
class STELLARSTRATAGEM_API UOnlineGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ConnectToServer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsConnected();
};
