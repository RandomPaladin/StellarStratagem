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

	bool TryCreateGame(AStellarPlayerController* Player, const FString& GameCode); //Returns true if game was created successfully
	bool TryJoinGame(AStellarPlayerController* Player, const FString& GameCode); //Returns true if game was joined successfully
	void TryLeaveGame(AStellarPlayerController* Player);

	UFUNCTION()
	void OnGameSpawnComplete(AGameManager* Game) const;

	AGameManager* GetGame(const FString& GameCode) const { return Games[GameCode]; }
	
	UPROPERTY(BlueprintAssignable)
	FOnGameCreatedDelegate OnGameCreatedOrJoined;
};
