#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ServerManager.generated.h"

class AStellarPlayerController;
class AGameManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameCreatedDelegate, AGameManager*, Game);

UCLASS()
class STELLARSTRATAGEM_API AServerManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TMap<FString, AGameManager*> Games;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AGameManager> GameManagerTemplate;

public:
	AServerManager();
	virtual void BeginPlay() override;

	void TryCreateGame(AStellarPlayerController* Player, const FString& GameCode);
	void TryJoinGame(AStellarPlayerController* Player, const FString& GameCode);
	void TryLeaveGame(AStellarPlayerController* Player);

	UFUNCTION()
	void OnGameSpawnComplete(AGameManager* Game) const;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameCreatedDelegate OnGameCreatedOrJoined;
};
