#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarPlayerController.generated.h"

class AShipAttackLine;
class APlanet;
class AGameManager;
class USpringArmComponent;
class AServerManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlanetSelectedDelegate, APlanet*, Planet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceivedDelegate, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldUpdatedDelegate, int, NewGoldAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShipAttackLineCreatedDelegate, AShipAttackLine*, ShipAttackLine);

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

	bool IsValid() const { return !Username.IsEmpty(); }
	
	bool operator ==(const FPlayerData& Other) const
	{
		return Username == Other.Username;
	}

	bool operator !=(const FPlayerData& Other) const
	{
		return Username != Other.Username;
	}
};

FORCEINLINE uint32 GetTypeHash(const FPlayerData& PlayerData)
{
	return GetTypeHash(PlayerData.Username);
}

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
	FPlayerData PlayerData;

	UPROPERTY()
	AGameManager* GameManager;

	//Player vars
	UPROPERTY(EditAnywhere)
	int StartingGold = 100;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GoldAmount)
	int GoldAmount = 0;
	UPROPERTY(VisibleAnywhere, Replicated)
	float TechLevel = 0.f;

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

	UPROPERTY(EditAnywhere)
	TSubclassOf<AShipAttackLine> ShipAttackLineTemplate;
	UPROPERTY(VisibleAnywhere)
	AShipAttackLine* CurrentShipAttackLine;
	UPROPERTY(VisibleAnywhere)
	TArray<AShipAttackLine*> ShipAttackLines;

private:
	//Replication funcs
	UFUNCTION()
	void OnRep_GoldAmount() const;
	
	//Game setup
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryCreateGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryJoinGame_Server(const FString& GameCode);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryLeaveGame_Server();
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TryStartGame_Server();

	//Actions
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SendAction_Server(const FActionData& ActionData);
	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ReceiveActionResult_Client(const FActionResult& ActionResult);
	
	//Other funcs
	UFUNCTION(BlueprintCallable)
	void CloseApplication();
	UFUNCTION(BlueprintCallable)
	void GoToMainMenu();
	AGameManager* GetGameManager();
	FVector ScreenToWorldLoc(const FVector& ScreenLoc) const;
	FVector ScreenToWorldDelta(const FVector& ScreenDelta) const;
	APlanet* GetHoveredPlanet(const FVector& ScreenLoc);

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
	void RemoveGold(int Gold);
	void AddTechXP(float Xp);

	UFUNCTION(BlueprintCallable)
	void CancelShipAttackLine(AShipAttackLine* AttackLine);

	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FPlayerData GetPlayerData() const { return PlayerData; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetSelectedPlanet();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetGold() const { return GoldAmount; }
	int GetTechLevel() const { return FMath::Floor(TechLevel); }

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnPlanetSelectedDelegate OnPlanetSelected;
	UPROPERTY(BlueprintAssignable)
	FOnMessageReceivedDelegate OnMessageReceived;
	UPROPERTY(BlueprintAssignable)
	FOnGoldUpdatedDelegate OnGoldUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnShipAttackLineCreatedDelegate OnShipAttackLineCreated;
};
