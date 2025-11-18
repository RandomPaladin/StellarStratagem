#pragma once

#include "CoreMinimal.h"
#include "Algo/Count.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Actions/BuildingType.h"
#include "StellarStratagem/Data/PlanetGradeData.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "StellarStratagem/Utility/HelperFunctions.h"
#include "Planet.generated.h"

class UShipData;
class UDataTable;
class AGameManager;
class UPlanetGradeData;

USTRUCT(BlueprintType)
struct FBuildingSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EBuildingType> CurrentBuildingType = BuildingType_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EBuildingType> TargetBuildingType = BuildingType_None;
	
	FBuildingSlot()
	{
		CurrentBuildingType = BuildingType_None;
		TargetBuildingType = BuildingType_None;
	}
};

USTRUCT(BlueprintType)
struct FShipAttackLineData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FPlayerData Player;
	UPROPERTY(VisibleAnywhere)
	int FromPlanetIndex;
	UPROPERTY(VisibleAnywhere)
	int ShipAmount;
	UPROPERTY(VisibleAnywhere)
	float Progress;
	
	FShipAttackLineData()
	{
		Player = {};
		FromPlanetIndex = 0;
		ShipAmount = 0;
		Progress = 0.f;
	}

	FShipAttackLineData(const FPlayerData& InPlayer, const int InFromPlanetIndex, const int InShipAmount)
	{
		Player = InPlayer;
		FromPlanetIndex = InFromPlanetIndex;
		ShipAmount= InShipAmount;
		Progress = 0.f;
	}
};

UCLASS()
class STELLARSTRATAGEM_API APlanet : public AActor
{
	GENERATED_BODY()

	//Setup
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;
	UPROPERTY()
	AGameManager* GameManager;
	UPROPERTY(VisibleAnywhere, Replicated)
	FPlayerData OwningPlayer;

	//Data
	UPROPERTY(EditAnywhere)
	UPlanetGradeData* PlanetData;
	UPROPERTY(EditAnywhere)
	UDataTable* NamesData;
	UPROPERTY(EditAnywhere)
	UShipData* ShipData;

	//Values
	UPROPERTY(VisibleAnywhere, Replicated)
	int PlanetIndex = -1;
	UPROPERTY(VisibleAnywhere, Replicated)
	FString PlanetName;
	UPROPERTY(VisibleAnywhere, Replicated)
	TEnumAsByte<EPlanetGrade> Grade;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_BuildingSlots)
	TArray<FBuildingSlot> BuildingSlots;
	UPROPERTY(VisibleAnywhere, Replicated)
	float ProductionDistribution = 0.5f;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ShipAmount)
	float ShipAmount = 0.f;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_IncomingAttackLines)
	TArray<FShipAttackLineData> IncomingAttackLines;

	//Art
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> PlanetMeshes;

	UFUNCTION(NetMulticast, Reliable)
	void Setup_Client(int MeshIndex);

	//Replication funcs
	UFUNCTION()
	void OnRep_BuildingSlots() const;
	UFUNCTION()
	void OnRep_ShipAmount() const;
	UFUNCTION()
	void OnRep_IncomingAttackLines() const;

public:
	APlanet();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Setup(AGameManager* Game, int Index);
	void SetOwningPlayer(const FPlayerData& NewOwningPlayer);
	void UpdateBuilding(AStellarPlayerController* Player, const int BuildingSlotIndex, const EBuildingType TargetBuildingType);
	void UpdateProductionDistribution(float NewDistribution);
	void AddIncomingAttackLine(const FPlayerData& InPlayer, const int InFromPlanetIndex, const int InShipAmount);
	
	int GetGeneratedGoldAmount() const;
	float GetGeneratedTechXPAmount() const;
	float GenerateShips();
	void ResolveBuildingPlans(OUT TArray<TTuple<bool, EBuildingType>>& Results); //bool: If building was built or destroyed, BuildingType: Building type that was built or destroyed
	void ResolveShipMovement();
	
	//Getters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AGameManager* GetGameManager() const { return GameManager; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetPlanetIndex() const { return PlanetIndex; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsOwnedByPlayer(const FPlayerData& PlayerData) const { return OwningPlayer == PlayerData; }
	bool IsOwnedByAnyPlayer() const { return OwningPlayer.IsValid(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FPlayerData GetOwningPlayer() const { return OwningPlayer; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TEnumAsByte<EPlanetGrade> GetGrade() const { return Grade; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetPlanetName() const { return PlanetName; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FBuildingSlot> GetBuildingSlots() const { return BuildingSlots; }
	UPlanetGradeData* GetPlanetData() const { return PlanetData; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FBuildingSlot GetBuildingSlot(const int BuildingSlotIndex) const { return BuildingSlots[BuildingSlotIndex]; }
	int GetBuildingAmount(EBuildingType BuildingType) const { return Algo::CountIf(BuildingSlots, [BuildingType](const FBuildingSlot& BuildingSlot){ return BuildingSlot.CurrentBuildingType == BuildingType; }); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetShipAmount() const { return ShipAmount; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FShipAttackLineData> GetIncomingAttackLines() const { return IncomingAttackLines; }

	int GetDistanceToPlanet(const APlanet* OtherPlanet) const;
	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnBuildingSlotsUpdated;
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnShipAmountUpdated;
	UPROPERTY(BlueprintAssignable)
	FNoParamDelegate OnIncomingAttackLinesUpdated;
};
