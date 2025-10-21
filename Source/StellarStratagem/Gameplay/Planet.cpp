#include "Planet.h"
#include "GameManager.h"
#include "StellarStratagem/Data/PlanetGradeData.h"

APlanet::APlanet()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool APlanet::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if(!GameManager)
		return false;

	return GameManager->GetConnectedPlayers().Contains(RealViewer);
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
}
