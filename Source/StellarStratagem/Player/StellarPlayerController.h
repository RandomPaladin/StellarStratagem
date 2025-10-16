#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class AServerManager;

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Username;
	
	FPlayerData()
	{
		Username = "USERNAMENOTSET";
	}

	FPlayerData(FString InUsername)
	{
		Username = InUsername;
	}

	bool operator ==(const FPlayerData& item1, const FPlayerData& item2) const
	{
		return item1.Username == item2.Username;
	}
};

UCLASS()
class STELLARSTRATAGEM_API AStellarPlayerController : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FString Username;

	UPROPERTY()
	AServerManager* ServerManager;

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void SendAction_Server(const FActionData& ActionData);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryConnectToGame_Server(const FString& GameCode);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetUsername() { return Username; }
};
