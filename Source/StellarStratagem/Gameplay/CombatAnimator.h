#pragma once

#include "CoreMinimal.h"
#include "GameManager.h"
#include "GameFramework/Actor.h"
#include "CombatAnimator.generated.h"

UCLASS()
class STELLARSTRATAGEM_API ACombatAnimator : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	AStellarPlayerController* LocalPlayer;
	UPROPERTY(BlueprintReadOnly)
	AGameManager* GameManager;
private:
	TArray<FRoundResolutionResult> Results;
	int ResultIndex = -1;

public:
	UFUNCTION(BlueprintCallable)
	void DoCombatAnimations();
	
	ACombatAnimator();
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DoCombatAnimation(const FCombatResult& CombatResult);
	UFUNCTION(BlueprintImplementableEvent)
	void ToggleUIUsability(bool Usable);
	UFUNCTION(BlueprintCallable)
	void IncrementAnimation();
};
