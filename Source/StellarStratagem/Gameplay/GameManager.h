#pragma once

#include "CoreMinimal.h"
#include "GameManager.generated.h"

class UActionBase;

UCLASS()
class STELLARSTRATAGEM_API AGameManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int Turn;

	UPROPERTY(VisibleAnywhere)
	TArray<APlayerController*> Players;

	UPROPERTY(VisibleAnywhere)
	TArray<APlayerController*> AwaitedPlayers;

public:
	AGameManager();
protected:
	virtual void BeginPlay() override;
};
