#include "CancelAttackLineAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult UCancelAttackLineAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	//Ensure this is only done on the server
	if(!GameManager->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO CANCEL ATTACK LINE OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}

	TArray<APlanet*> Planets = GameManager->GetPlanets();

	//Ensure ship line is valid
	if(!Data.ShipAttackLine.IsValid(GameManager))
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID SHIP LINE"))
		return {false, "Invalid from planet given."};
	}

	//Ensure target planet index is valid
	if(Data.IntValue < 0 || Data.IntValue >= Planets.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID TO PLANET INDEX"))
		return {false, "Invalid target planet given."};
	}
	
	const APlanet* FromPlanet = Planets[Data.ShipAttackLine.FromPlanetIndex];
	APlanet* ToPlanet = Planets[Data.IntValue];

	//Ensure player owns from planet
	if(FromPlanet->GetOwningPlayer() != Player->GetPlayerData())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN PLAYER DOES NOT OWN FROM PLANET"))
		return {false, "You do not own the starting planet."};
	}

	//Ensure to planet actually contains given ship line
	if(!ToPlanet->GetIncomingAttackLines().Contains(Data.ShipAttackLine))
	{
		UE_LOG(LogTemp, Error, TEXT("TARGET PLANET DOESN'T CONTAIN THIS INCOMING ATTACK LINE"))
		return {false, ""};
	}

	//Set attack line
	ToPlanet->RemoveIncomingAttackLine(Data.ShipAttackLine);
	return {true, ""};
}
