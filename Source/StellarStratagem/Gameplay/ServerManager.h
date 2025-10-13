#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ServerManager.generated.h"

class AStellarPlayerController;
class AGameManager;

UCLASS()
class STELLARSTRATAGEM_API AServerManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, AGameManager*> Games;

	UPROPERTY()
	TSubclassOf<AGameManager> GameManagerTemplate;

public:
	AServerManager();
	virtual void BeginPlay() override;

	void TryConnectToGame(AStellarPlayerController* Player, const FString& GameCode);
};
