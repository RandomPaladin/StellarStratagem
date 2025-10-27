#include "Planet.h"
#include "GameManager.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Data/PlanetBuildingsData.h"
#include "StellarStratagem/Data/PlanetGradeData.h"
#include "StellarStratagem/Data/PlanetNamesDataTable.h"

APlanet::APlanet()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	//Create mesh comp
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("PlanetMesh");
	RootComponent = MeshComp;
}

bool APlanet::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if(!GameManager)
		return true;

	return GameManager->GetConnectedPlayers().Contains(RealViewer);
}

void APlanet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlanet, OwningPlayer);
	DOREPLIFETIME(APlanet, PlanetIndex);
	DOREPLIFETIME(APlanet, PlanetName);
	DOREPLIFETIME(APlanet, Grade);
	DOREPLIFETIME(APlanet, BuildingSlots);
	DOREPLIFETIME(APlanet, ProductionDistribution);
	DOREPLIFETIME(APlanet, ShipAmount);
}

void APlanet::Setup(AGameManager* Game, const int Index)
{
	//Ensure setup only occurs on server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO SET UP PLANET OUTSIDE OF SERVER"))
		return;
	}
	
	//Record game manager + index
	GameManager = Game;
	PlanetIndex = Index;

	//Set random name
	TArray<FPlanetNamesDataTable*> Rows;
	NamesData->GetAllRows<FPlanetNamesDataTable>("", Rows);
	PlanetName = Rows[FMath::RandRange(0, Rows.Num() - 1)]->PlanetName;
	
	//Set random grade
	TArray<TEnumAsByte<EPlanetGrade>> AllGradesInData;
	GradesData->Grades.GetKeys(AllGradesInData);
	Grade = AllGradesInData[FMath::RandRange(0, AllGradesInData.Num() - 1)];

	//Set random building slots
	BuildingSlots.Empty();
	const int SlotAmount = GradesData->GenerateRandomBuildingSlotAmount(Grade);
	for(int i = 0; i < SlotAmount; i++)
		BuildingSlots.Add({});

	//Setup on clients
	Setup_Client(FMath::RandRange(0, PlanetMeshes.Num() - 1));
}

void APlanet::SetOwningPlayer(const FPlayerData& NewOwningPlayer)
{
	OwningPlayer = NewOwningPlayer;
}

void APlanet::Setup_Client_Implementation(const int MeshIndex)
{
	//Ignore client stuff on server
	if(HasAuthority())
		return;

	//Set new mesh
	MeshComp->SetStaticMesh(PlanetMeshes[MeshIndex]);

	//Get game manager
	GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	//Register self in game manager
	GameManager->RegisterPlanet(this);
}

void APlanet::UpdateBuilding(AStellarPlayerController* Player, const int BuildingSlotIndex, const EBuildingType TargetBuildingType)
{
	FBuildingSlot* Slot = &BuildingSlots[BuildingSlotIndex];
	
	const bool HasCurrentBuilding = Slot->CurrentBuildingType != BuildingType_None;
	const bool HasPlannedBuilding = Slot->TargetBuildingType != BuildingType_None;
	const bool HasTargetBuilding = TargetBuildingType != BuildingType_None;
	
	//Handle building update
	if(!HasCurrentBuilding && HasTargetBuilding) //Plan new building
	{
		//Remove gold
		Player->RemoveGold(BuildingsData->Buildings[TargetBuildingType].GoldCost);

		//Plan build
		Slot->TargetBuildingType = TargetBuildingType;
	}
	else if(!HasCurrentBuilding && HasPlannedBuilding) //Remove plan for new building
	{
		//Give gold back
		Player->AddGold(BuildingsData->Buildings[Slot->TargetBuildingType].GoldCost);

		//Unplan build
		Slot->TargetBuildingType = BuildingType_None;
	}
	else if(HasCurrentBuilding && !HasTargetBuilding) //Mark for destruction
		Slot->TargetBuildingType = BuildingType_None;
	else if (HasCurrentBuilding && !HasPlannedBuilding) //Unmark for destruction
		Slot->TargetBuildingType = Slot->CurrentBuildingType;
}

void APlanet::UpdateProductionDistribution(const float NewDistribution)
{
	ProductionDistribution = NewDistribution;
}

#pragma region Replication Funcs

void APlanet::OnRep_BuildingSlots() const
{
	OnBuildingSlotsUpdated.Broadcast();
}

void APlanet::OnRep_ShipAmount() const
{
	OnShipAmountUpdated.Broadcast();
}

#pragma endregion

int APlanet::GetGeneratedGoldAmount() const
{
	//Multiply with gold per factory from grades data to get gold amount
	int GoldAmount = GetFactoryAmount() * GradesData->Grades[Grade].GoldPerFactoryPerRound;

	//Apply production distribution
	GoldAmount *= ProductionDistribution;
	
	return GoldAmount;
}

float APlanet::GenerateShips()
{
	//Multiply with ships per factory from grades data to get ship amount
	float GeneratedShipAmount = (float)GetFactoryAmount() * GradesData->Grades[Grade].ShipsPerFactoryPerRound;
	
	//Apply production distribution
	GeneratedShipAmount *= 1.f - ProductionDistribution;

	//Update ship amount
	ShipAmount += GeneratedShipAmount;
	
	return GeneratedShipAmount;
}

void APlanet::ResolveBuildingPlans(TArray<TTuple<bool, EBuildingType>>& Results)
{
	//Ensure given results array is empty
	Results.Empty();
	
	//Resolve
	for (FBuildingSlot& BuildingSlot : BuildingSlots)
	{
		//No plans set, ignore this slot
		if(BuildingSlot.CurrentBuildingType == BuildingSlot.TargetBuildingType)
			continue;

		//Resolve plan
		auto PreviousBuildingType = BuildingSlot.CurrentBuildingType;
		BuildingSlot.CurrentBuildingType = BuildingSlot.TargetBuildingType;

		//Add result to array
		bool Built = BuildingSlot.CurrentBuildingType != BuildingType_None;
		Results.Add(TTuple<bool, EBuildingType>{Built, Built ? BuildingSlot.CurrentBuildingType : PreviousBuildingType});
	}
}
