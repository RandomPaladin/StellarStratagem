#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineGameInstance.generated.h"

class IOnlineSubsystem;

UCLASS()
class STELLARSTRATAGEM_API UOnlineGameInstance : public UGameInstance
{
	GENERATED_BODY()

	IOnlineSubsystem* OnlineSub;
	IOnlineSessionPtr SessionInterface;

	FName TestSessionName = "TestSession";

	TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;

	FOnDestroySessionCompleteDelegate OnDestroyOldSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> LobbyMap;

	void EnsureSetup();

	void OnDestroyOldSessionComplete(FName SessionName, bool Successful);
	void OnFindSessionsComplete(bool Successful);

public:
	UFUNCTION(BlueprintCallable)
	void CreateSession();
	UFUNCTION(BlueprintCallable)
	void FindSessions();
};
