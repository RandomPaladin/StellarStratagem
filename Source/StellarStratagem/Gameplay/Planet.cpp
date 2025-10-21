#include "Planet.h"
#include "GameManager.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Data/PlanetGradeData.h"

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
	DOREPLIFETIME(APlanet, Grade);
	DOREPLIFETIME(APlanet, IndustrialBuildings);
	DOREPLIFETIME(APlanet, ResearchBuildings);
}

void APlanet::Setup(AGameManager* Game)
{
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO SET UP PLANET OUTSIDE OF SERVER"))
		return;
	}
	
	//Record game manager
	GameManager = Game;
	
	//Set random grade
	TArray<TEnumAsByte<EPlanetGrade>> AllGradesInData;
	GradesData->Grades.GetKeys(AllGradesInData);
	TEnumAsByte<EPlanetGrade> RandomGrade = AllGradesInData[FMath::RandRange(0, AllGradesInData.Num() - 1)];
	Grade = RandomGrade;

	UE_LOG(LogTemp, Warning, TEXT("SETUP PLANET"))

	//Send random mesh index to clients
	SetPlanetMesh(FMath::RandRange(0, PlanetMeshes.Num() - 1));
}

void APlanet::SetOwningPlayer(const FPlayerData& NewOwningPlayer)
{
	OwningPlayer = NewOwningPlayer;
}

}
