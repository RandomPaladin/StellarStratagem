#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineGameInstance.generated.h"

class IOnlineSubsystem;

UCLASS()
class STELLARSTRATAGEM_API UOnlineGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> LobbyMap;

public:
	UFUNCTION(BlueprintCallable)
	void ConnectToServer();
};
