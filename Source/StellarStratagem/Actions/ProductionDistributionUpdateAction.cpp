#include "ProductionDistributionUpdateAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult UProductionDistributionUpdateAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	//Ensure given planet index is valid
	if(Data.IntValue < 0 || Data.IntValue >= GameManager->GetPlanets().Num())
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID PLANET INDEX"))
		return {false, "Invalid planet given."};
	}
	
	APlanet* Planet = GameManager->GetPlanets()[Data.IntValue];

	//Ensure this is only done on the server
	if(!Planet->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO UPDATE PRODUCTION DISTRIBUTION OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}

	//Ensure player owns this planet
	if(Planet->GetOwningPlayer() != Player->GetPlayerData())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN PLAYER DOES NOT OWN THIS PLANET"))
		return {false, "You do not own this planet."};
	}

	//Update production distribution
	Planet->UpdateProductionDistribution(FMath::Clamp(Data.NumberValue, 0.f, 1.f));
	return {true, ""};
}
