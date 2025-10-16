#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "GameManager.generated.h"

class AServerManager;
class AStellarPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayersUpdatedDelegate);

UCLASS()
class STELLARSTRATAGEM_API AGameManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	AServerManager* ServerManager;

	UPROPERTY(VisibleAnywhere)
	int Turn;

	UPROPERTY(VisibleAnywhere)
	TMap<AActor*, AStellarPlayerController*> ConnectedPlayers;

	UPROPERTY()
	bool GameStarted = false;
	
	UFUNCTION()
	void OnRep_ConnectedPlayers() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ConnectedPlayers)
	TArray<FPlayerData> AllPlayers;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPlayerData> AwaitedPlayers;

public:
	AGameManager();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	void AddPlayer(AStellarPlayerController* Player);
	void RemovePlayer(AStellarPlayerController* Player);

	TArray<AStellarPlayerController*> GetConnectedPlayers() const;
	bool GetGameStarted() const { return GameStarted; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerPartOfGame(FString Username) const { return AllPlayers.FindByPredicate([Username](const FPlayerData& Player){ return Player.Username == Username; }) != nullptr; }

	UPROPERTY(BlueprintAssignable)
	FOnPlayersUpdatedDelegate OnPlayersUpdated;
};
