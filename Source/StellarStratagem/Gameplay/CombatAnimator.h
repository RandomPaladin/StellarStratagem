#pragma once

#include "CoreMinimal.h"
#include "GameManager.h"
#include "GameFramework/Actor.h"
#include "CombatAnimator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIsBusyUpdatedDelegate, bool, NewIsBusy);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsBusy = false;
	
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

public:
	UPROPERTY(BlueprintAssignable)
	FOnIsBusyUpdatedDelegate OnIsBusyUpdated;
};
