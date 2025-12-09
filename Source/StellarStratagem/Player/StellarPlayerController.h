#pragma once

#include "CoreMinimal.h"
#include "PlayerData.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class AShipAttackLineManager;
class AShipAttackLine;
class APlanet;
class AGameManager;
class USpringArmComponent;
class AServerManager;
struct FActionData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlanetSelectedDelegate, APlanet*, Planet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceivedDelegate, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipAttackLineCreatedDelegate, AShipAttackLine*, ShipAttackLine);

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
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_PlayerDataIndex)
	int PlayerDataIndex = -1;

	UPROPERTY()
	AGameManager* GameManager;

	UPROPERTY(VisibleAnywhere)
	AShipAttackLineManager* ShipAttackLineManager;

	FString LocalUsername;

	//Input vars
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* CamActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USpringArmComponent* CamSpringArm;
	FVector StartTouchLoc;
	FVector CurrentTouchLoc;
	FVector PreviousTouchLoc;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool TwoFingersTouching = false;
	UPROPERTY(VisibleAnywhere)
	bool DraggingFromPlanet = false;
	UPROPERTY(EditAnywhere)
	float PlanetSelectRadius = 200.f; //How close the player needs to click to a planet to count as selecting it
	float PlanetSelectRadiusSqr;

	UPROPERTY()
	APlanet* InitiallyPressedPlanet;
	UPROPERTY()
	APlanet* SelectedPlanet;
	
	UPROPERTY(VisibleAnywhere)
	AShipAttackLine* CurrentShipAttackLine;

	UPROPERTY(VisibleAnywhere)
	TArray<UObject*> InputOccluders;

private:
	//Game setup
	UFUNCTION(BlueprintCallable)
	void TryCreateGame(const FString& GameCode);
	UFUNCTION(BlueprintCallable)
	void TryJoinGame(const FString& GameCode);
	UFUNCTION(Server, Reliable)
	void TryCreateGame_Server(const FString& GameCode);
	UFUNCTION(Server, Reliable)
	void TryJoinGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryLeaveGame_Server();
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryStartGame_Server();

public:
	UFUNCTION(Client, Reliable)
	void AskForPlayerData_Client();
private:
	UFUNCTION(Server, Reliable)
	void SendPlayerData_Server(const FPlayerData& PlayerData);

	UFUNCTION()
	void OnRep_PlayerDataIndex();

	//Actions
public:
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SendAction_Server(const FActionData& ActionData);
private:
	UFUNCTION(Client, Reliable)
	void ReceiveActionResult_Client(const FActionResult& ActionResult);
	
	//Other funcs
	UFUNCTION(BlueprintCallable)
	void CloseApplication();
	UFUNCTION(BlueprintCallable)
	void GoToMainMenu();
	AGameManager* GetGameManager();
	AShipAttackLineManager* GetShipAttackLineManager();
	FVector ScreenToWorldLoc(const FVector& ScreenLoc) const;
	FVector ScreenToWorldDelta(const FVector& ScreenDelta) const;
	APlanet* GetHoveredPlanet(const FVector& ScreenLoc);

	UFUNCTION(BlueprintCallable)
	void ShowMessage(FString Message) const;

	//Input funcs
	UFUNCTION(BlueprintCallable)
	void OnPress(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressMoved(const FVector& Loc);
	UFUNCTION(BlueprintCallable)
	void OnPressReleased(const FVector& Loc);

	bool IsInputOccluded() const { return InputOccluders.Num() == 0; }
public:
	UFUNCTION(BlueprintCallable)
	void UpdateInputOcclusion(UObject* Occluder, bool Occluding);
	
	//Other funcs
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetPlayerDataIndex(int NewIndex);

	UFUNCTION(BlueprintCallable)
	void SetLocalUsername(FString NewLocalUsername);

	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FPlayerData GetPlayerData();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetSelectedPlanet();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetGold() { return GetPlayerData().GoldAmount; }
	FString GetCurrentGameCode() const { return CurrentGameCode; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetLocalUsername() const { return LocalUsername; }

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnPlanetSelectedDelegate OnPlanetSelected;
	UPROPERTY(BlueprintAssignable)
	FOnMessageReceivedDelegate OnMessageReceived;
	UPROPERTY(BlueprintAssignable)
	FOnShipAttackLineCreatedDelegate OnShipAttackLineCreated;
};
