#include "ShipAttackLineManager.h"
#include "GameManager.h"
#include "Planet.h"
#include "ShipAttackLine.h"
#include "Kismet/GameplayStatics.h"

AShipAttackLineManager::AShipAttackLineManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShipAttackLineManager::BeginPlay()
{
	Super::BeginPlay();
	
	//Get local player
	LocalPlayer = Cast<AStellarPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	//Get game manager
	GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	//Ignore this class on the server
	if(GameManager->HasAuthority())
		return;
	
	//Bind to game manager
	GameManager->OnPlanetListUpdated.AddDynamic(this, &AShipAttackLineManager::SetupShipAttackLineManager);
	SetupShipAttackLineManager();
}

void AShipAttackLineManager::SetupShipAttackLineManager()
{
	for (APlanet* Planet : GameManager->GetPlanets())
		Planet->OnIncomingAttackLinesUpdated.AddUniqueDynamic(this, &AShipAttackLineManager::OnIncomingAttackLinesUpdated);
}

void AShipAttackLineManager::OnIncomingAttackLinesUpdated()
{
	//Remove old lines
	if(ShipAttackLines.Num() > 0)
	{
		for(int i = ShipAttackLines.Num() - 1; i > -1; i--)
			ShipAttackLines[i]->Destroy();

		ShipAttackLines.Empty();
	}

	//Create new lines
	for (APlanet* Planet : GameManager->GetPlanets())
	{
		TArray<FShipAttackLineData> IncomingAttackLines = Planet->GetIncomingAttackLines();
		for (const FShipAttackLineData& IncomingAttackLine : IncomingAttackLines)
		{
			//Only show local player's attack lines
			APlanet* FromPlanet = GameManager->GetPlanets()[IncomingAttackLine.FromPlanetIndex];
			if(!FromPlanet->IsOwnedByPlayer(LocalPlayer->GetPlayerData()))
				continue;

			AShipAttackLine* Line = CreateAttackLine();
			Line->SetupAttackLine(LocalPlayer, FromPlanet, Planet, IncomingAttackLine.ShipAmount);
		}
	}
}

AShipAttackLine* AShipAttackLineManager::CreateAttackLine()
{
	AShipAttackLine* AttackLine = GetWorld()->SpawnActor<AShipAttackLine>(ShipAttackLineTemplate, FVector::ZeroVector, FRotator::ZeroRotator);
	AttackLine->SetFromLoc(GetActorLocation());
	ShipAttackLines.Add(AttackLine);
	return AttackLine;
}

void AShipAttackLineManager::RemoveAttackLine(AShipAttackLine* AttackLine)
{
	ShipAttackLines.Remove(AttackLine);
	AttackLine->Destroy();
}

void AShipAttackLineManager::CancelShipAttackLine(AShipAttackLine* AttackLine)
{
	if(!ShipAttackLines.Contains(AttackLine))
		return;

	//Send cancel ship line action to server
	FActionData CancelAttackLineAction = {};
	CancelAttackLineAction.ActionType = ActionType_CancelAttackLine;
	CancelAttackLineAction.ShipAttackLine = FShipAttackLineData{LocalPlayer->GetPlayerData(), AttackLine->GetFromPlanet()->GetPlanetIndex(), AttackLine->GetShipAmount()};
	CancelAttackLineAction.IntValue = AttackLine->GetTargetPlanet()->GetPlanetIndex();
	LocalPlayer->SendAction_Server(CancelAttackLineAction);

	//Remove line
	ShipAttackLines.Remove(AttackLine);
	AttackLine->Destroy();
}

AShipAttackLine* AShipAttackLineManager::GetShipAttackLineToPlanet(const APlanet* TargetPlanet) const
{
	AShipAttackLine* const* AttackLinePtr = ShipAttackLines.FindByPredicate([TargetPlanet](const AShipAttackLine* AttackLine) { return AttackLine->GetTargetPlanet() == TargetPlanet; });
	if(!AttackLinePtr)
		return nullptr;

	return *AttackLinePtr;
}