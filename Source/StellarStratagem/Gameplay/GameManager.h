#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "GameManager.generated.h"

class APlanet;
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

	//Planets
	UPROPERTY(EditAnywhere, Category=Planets)
	TSubclassOf<APlanet> PlanetTemplate;
	UPROPERTY(EditAnywhere, Category=Planets)
	int SpawnPlanetsPerPlayer = 5;
	UPROPERTY(EditAnywhere, Category=Planets)
	FVector2D SpawnPlanetXLocRange = {-3000.f, 3000.f};
	UPROPERTY(EditAnywhere, Category=Planets)
	FVector2D SpawnPlanetYLocRange = {-3000.f, 3000.f};
	UPROPERTY(EditAnywhere, Category=Planets)
	FVector2D SpawnPlanetRotRange = {0.f, 359.f};
	UPROPERTY(VisibleAnywhere, Category=Planets)
	TArray<APlanet*> Planets;

	//Gameplay
private:
	void GoToNextRound();
public:
	void RegisterPlanet(APlanet* Planet);

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
	TMap<AActor*, AStellarPlayerController*> GetConnectedPlayers() const { return ConnectedPlayers; }
	TArray<AStellarPlayerController*> GetConnectedPlayerControllers() const;
	int GetPlayerAmount() const { return AllPlayers.Num(); }
	bool GetGameStarted() const { return GameStarted; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerPartOfGame(FString Username) const { return AllPlayers.FindByPredicate([Username](const FPlayerData& Player){ return Player.Username == Username; }) != nullptr; }

	TArray<APlanet*> GetPlanets() const { return Planets; }
	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnPlayersUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnGameStateChanged OnGameStateUpdated;
};
