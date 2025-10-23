#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Actions/BuildingType.h"
#include "StellarStratagem/Data/PlanetGradeData.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "Planet.generated.h"

class UPlanetBuildingsData;
class UDataTable;
class AGameManager;
class UPlanetGradeData;

USTRUCT(BlueprintType)
struct FBuildingSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EBuildingType> BuildingType;
	UPROPERTY(BlueprintReadOnly)
	bool MarkedForDestroy;
	
	FBuildingSlot()
	{
		BuildingType = BuildingType_None;
		MarkedForDestroy = false;
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
	UPlanetGradeData* GradesData;
	UPROPERTY(EditAnywhere)
	UDataTable* NamesData;
	UPROPERTY(EditAnywhere)
	UPlanetBuildingsData* BuildingsData;

	//Values
	UPROPERTY(VisibleAnywhere, Replicated)
	int PlanetIndex = -1;
	UPROPERTY(VisibleAnywhere, Replicated)
	FString PlanetName;
	UPROPERTY(VisibleAnywhere, Replicated)
	TEnumAsByte<EPlanetGrade> Grade;
	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<FBuildingSlot> BuildingSlots;

	//Art
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> PlanetMeshes;

	UFUNCTION(NetMulticast, Reliable)
	void Setup_Client(int MeshIndex);

public:
	APlanet();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Setup(AGameManager* Game, int Index);
	void SetOwningPlayer(const FPlayerData& NewOwningPlayer);
	void Build(AStellarPlayerController* Player, const int BuildingSlotIndex, const EBuildingType BuildingType);
	int GetGeneratedGoldAmount();
	float GenerateShips();

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
	UPlanetBuildingsData* GetBuildingsData() const { return BuildingsData; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FBuildingSlot GetBuildingSlot(const int BuildingSlotIndex) const { return BuildingSlots[BuildingSlotIndex]; }
};
