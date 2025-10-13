#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

class AStellarPlayerController;

UCLASS()
class STELLARSTRATAGEM_API AGameManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int Turn;

	UPROPERTY(VisibleAnywhere)
	TMap<AActor*, AStellarPlayerController*> Players;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> AwaitedPlayers;

public:
	AGameManager();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void BeginPlay() override;

	void SetupGame();
	void AddPlayer(AStellarPlayerController* Player);

	auto GetPlayers() { return Players; }
};
