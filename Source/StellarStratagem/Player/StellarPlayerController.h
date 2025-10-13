#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

UCLASS()
class STELLARSTRATAGEM_API AStellarPlayerController : public APlayerController
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void SendAction(const FActionData& ActionData);

	UFUNCTION(Client, Reliable)
	void Receive();
};
