#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class APlanet;
class AGameManager;
class USpringArmComponent;
class AServerManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlanetSelectedDelegate, APlanet*, Planet);

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

	//Setup vars
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> MainGameMap;
	UPROPERTY()
	AServerManager* ServerManager;
	UPROPERTY()
	FString CurrentGameCode;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	FString Username;

	UPROPERTY()
	AGameManager* GameManager;

	//Player vars
	UPROPERTY(VisibleAnywhere, Replicated)
	int GoldAmount = 0;

	//Input vars
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
	UPROPERTY(EditAnywhere)
	float PlanetSelectRadius = 200.f; //How close the player needs to click to a planet to count as selecting it
	float PlanetSelectRadiusSqr;
	UPROPERTY()
	APlanet* SelectedPlanet;

private:
	//Server RPCs
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
	
	//Other funcs
	UFUNCTION(BlueprintCallable)
	void CloseApplication();
	UFUNCTION(BlueprintCallable)
	void GoToMainMenu();
	AGameManager* GetGameOnClient();
	FVector ScreenToWorldLoc(const FVector& ScreenLoc) const;

	//Input funcs
	UFUNCTION(BlueprintCallable)
	void OnPress(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressMoved(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressReleased(const FVector& Loc);

	//Public
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddGold(int Gold);

	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetUsername() { return Username; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetSelectedPlanet();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetGold() const { return GoldAmount; }

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnPlanetSelectedDelegate OnPlanetSelected;
};
