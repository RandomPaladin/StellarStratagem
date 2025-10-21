#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StellarStratagem/Player/StellarPlayerController.h"
#include "Planet.generated.h"

class AGameManager;
enum EPlanetGrade : int;
class UPlanetGradeData;

UCLASS()
class STELLARSTRATAGEM_API APlanet : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;
	UPROPERTY()
	AGameManager* GameManager;

	UPROPERTY(VisibleAnywhere, Replicated)
	FPlayerData OwningPlayer;

	UPROPERTY(EditAnywhere)
	UPlanetGradeData* GradesData;
	UPROPERTY(VisibleAnywhere, Replicated)
	TEnumAsByte<EPlanetGrade> Grade;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	int IndustrialBuildings = 0;
	UPROPERTY(VisibleAnywhere, Replicated)
	int ResearchBuildings = 0;

	//Art
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> PlanetMeshes;

	UFUNCTION(NetMulticast, Reliable)
	void SetPlanetMesh(int MeshIndex);

public:
	APlanet();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Setup(AGameManager* Game);
	void SetOwningPlayer(const FPlayerData& NewOwningPlayer);
	bool IsOwnedByPlayer() const { return OwningPlayer.IsValid(); }
	TEnumAsByte<EPlanetGrade> GetGrade() const { return Grade; }
};
