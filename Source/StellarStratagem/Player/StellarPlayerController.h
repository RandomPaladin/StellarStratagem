#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class AServerManager;

UCLASS()
class STELLARSTRATAGEM_API AStellarPlayerController : public APlayerController
{
	GENERATED_BODY()

	FString Username;

	UPROPERTY()
	AServerManager* ServerManager;

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void SendAction(const FActionData& ActionData);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryConnectToGame(const FString& GameCode);

public:
	FString GetUsername() { return Username; }
};
