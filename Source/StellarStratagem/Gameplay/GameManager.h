#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "StellarStratagem/Utility/HelperFunctions.h"
#include "GameManager.generated.h"

class APlanet;
class AServerManager;
class AStellarPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, bool, Started);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlanetListUpdated);

UENUM(BlueprintType)
enum ERoundResolutionResultType
{
	RoundResolutionResultType_None,
	RoundResolutionResultType_Combat,
	RoundResolutionResultType_Resources,
};

USTRUCT(BlueprintType)
struct FCombatResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FPlayerData Attacker;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FPlayerData Defender;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int DefendingPlanetIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int AttackingShips;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int DefendingShips;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<bool> AttackerWon;

	FCombatResult()
	{
		Attacker = {};
		Defender = {};
		DefendingPlanetIndex = 0;
		AttackingShips = 0;
		DefendingShips = 0;
		AttackerWon = {};
	}

	FCombatResult(const FPlayerData& InAttacker, const FPlayerData& InDefender, const int InDefendingPlanetIndex, const int InAttackingShips, const int InDefendingShips, const TArray<bool>& InAttackerWon)
	{
		Attacker = InAttacker;
		Defender = InDefender;
		DefendingPlanetIndex = InDefendingPlanetIndex;
		AttackingShips = InAttackingShips;
		DefendingShips = InDefendingShips;
		AttackerWon = InAttackerWon;
	}
};

USTRUCT(BlueprintType)
struct FRoundResolutionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<ERoundResolutionResultType> ResultType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Result;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FCombatResult CombatResult;

	FRoundResolutionResult()
	{
		ResultType = RoundResolutionResultType_None;
		Result = "";
	}

	FRoundResolutionResult(const TEnumAsByte<ERoundResolutionResultType> InResultType, FString InResult)
	{
		ResultType = InResultType;
		Result = InResult;
	}
	
	FRoundResolutionResult(const TEnumAsByte<ERoundResolutionResultType> InResultType, FString InResult, const FCombatResult& InCombatResult)
	{
		ResultType = InResultType;
		Result = InResult;
		CombatResult = InCombatResult;
	}
};

USTRUCT(BlueprintType)
struct FRoundResolutionResults
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPlayerData Player;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FRoundResolutionResult> Results;

	FRoundResolutionResults()
	{
		Player = {};
		Results = {};
	}

	FRoundResolutionResults(const FPlayerData& InPlayerData)
	{
		Player = InPlayerData;
		Results = {};
	}
};

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
	UFUNCTION()
	void OnRep_PlayersResolutionResults() const;

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

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_PlayersResolutionResults)
	TArray<FRoundResolutionResults> PlayersResolutionResults;

	//Setup
public:
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
	AStellarPlayerController* GetPlayerControllerByPlayerData(const FPlayerData& PlayerData);
	TArray<FPlayerData> GetAwaitedPlayers() const { return AwaitedPlayers; }
	int GetPlayerAmount() const { return AllPlayers.Num(); }
	bool GetGameStarted() const { return GameStarted; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerPartOfGame(const FPlayerData& Player) const { return AllPlayers.FindByPredicate([Player](const FPlayerData& PlayerItem){ return PlayerItem == Player; }) != nullptr; }

	TArray<APlanet*> GetPlanets() const { return Planets; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<APlanet*> GetPlanetsOwnedByPlayer(const FPlayerData& Player);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetPlanetByName(const FString& InName);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APlanet* GetPlanetByIndex(const int PlanetIndex) const { return Planets[PlanetIndex]; }

	int GetIndexOfPlayersResolutionResults(const FPlayerData& Player) const { return PlayersResolutionResults.IndexOfByPredicate([Player](const FRoundResolutionResults& Results) { return Results.Player == Player; }); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRoundResolutionResults GetPlayerResolutionResults(const FPlayerData& Player) const { return PlayersResolutionResults[GetIndexOfPlayersResolutionResults(Player)]; }
	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnPlayersUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnGameStateChanged OnGameStateUpdated;
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnPlayersResolutionResultsUpdated;
	UPROPERTY(BlueprintAssignable)
	FOnPlanetListUpdated OnPlanetListUpdated;
};
