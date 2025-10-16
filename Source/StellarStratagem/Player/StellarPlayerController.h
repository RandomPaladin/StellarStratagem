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

	bool operator ==(const FPlayerData& Other) const
	{
		return Username == Other.Username;
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
	void TryCreateGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryJoinGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryLeaveGame_Server();

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetUsername() { return Username; }
};
