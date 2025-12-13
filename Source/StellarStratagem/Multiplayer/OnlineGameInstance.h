#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IHttpRequest.h"
#include "OnlineGameInstance.generated.h"

class AStellarPlayerController;
class IOnlineSubsystem;

UCLASS()
class STELLARSTRATAGEM_API UOnlineGameInstance : public UGameInstance
{
	GENERATED_BODY()

	FString ServerIpAddress = "192.168.0.192";
	FString ServerEndpointPort = "7777";

	void OnGameCreated(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success);

	UPROPERTY()
	AStellarPlayerController* InstigatingPlayer;

	void SendRequestToServer(const FString& RequestBody, const FString& FunctionName, void (UOnlineGameInstance::*Callback)(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success));
	
public:
	void CreateGame(AStellarPlayerController* Player, const FString& GameCode);
	void JoinGame(AStellarPlayerController* Player, const FString& GameCode);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsConnected() const;
};
