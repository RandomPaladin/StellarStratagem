#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class USpringArmComponent;
class AServerManager;

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Username;
	
	FPlayerData()
	{
		Username = "";
	}

	FPlayerData(FString InUsername)
	{
		Username = InUsername;
	}

	bool operator ==(const FPlayerData& Other) const
	{
		return Username == Other.Username;
	}

	bool IsValid() const { return !Username.IsEmpty(); }
};

UCLASS()
class STELLARSTRATAGEM_API AStellarPlayerController : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> MainGameMap;
	UPROPERTY()
	AServerManager* ServerManager;
	UPROPERTY()
	FString CurrentGameCode;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	FString Username;

	//Input
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* CamActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USpringArmComponent* CamSpringArm;
	FVector StartCamLoc;
	FVector StartTouchLoc;
	FVector CurrentTouchLoc;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool TwoFingersTouching = false;
	UPROPERTY(EditAnywhere)
	float ScrollAcceleration = 5.f;

private:
	//Funcs
	UFUNCTION(Server, Reliable)
	void SendAction_Server(const FActionData& ActionData);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryCreateGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryJoinGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryLeaveGame_Server();
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryStartGame_Server();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryEndTurn_Server();

	UFUNCTION(BlueprintCallable)
	void CloseApplication();
	UFUNCTION(BlueprintCallable)
	void GoToMainMenu();

	//Input
	UFUNCTION(BlueprintCallable)
	void OnPress(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressMoved(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressReleased(const FVector& Loc);

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetUsername() { return Username; }
};
