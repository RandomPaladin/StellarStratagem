#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "GameManager.generated.h"

class AServerManager;
class AStellarPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoParamDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, bool, Started);

UCLASS()
class STELLARSTRATAGEM_API AGameManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	AServerManager* ServerManager;

	UPROPERTY(VisibleAnywhere, Replicated)
	int Round = 0;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GameStarted)
	bool GameStarted = false;

	//Replication callbacks
	UFUNCTION()
	void OnRep_ConnectedPlayers() const;
	UFUNCTION()
	void OnRep_GameStarted() const;

	UPROPERTY(VisibleAnywhere)
	TMap<AActor*, AStellarPlayerController*> ConnectedPlayers;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ConnectedPlayers)
	TArray<FPlayerData> AllPlayers;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPlayerData> AwaitedPlayers;

public:
	//Setup
	AGameManager();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	//Lobby
	void AddPlayer(AStellarPlayerController* Player);
	void RemovePlayer(AStellarPlayerController* Player);

	//Game
	void StartGame();
	void EndTurn(AStellarPlayerController* Player);

	//Getters
	TArray<AStellarPlayerController*> GetConnectedPlayers() const;
	bool GetGameStarted() const { return GameStarted; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerPartOfGame(FString Username) const { return AllPlayers.FindByPredicate([Username](const FPlayerData& Player){ return Player.Username == Username; }) != nullptr; }

	//Delegates
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnPlayersUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnGameStateChanged OnGameStateUpdated;
};
