#include "SetAttackLineAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult USetAttackLineAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	//Ensure this is only done on the server
	if(!GameManager->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO SET ATTACK LINE OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}

	TArray<APlanet*> Planets = GameManager->GetPlanets();

	//Ensure given planet indexes are valid
	if(Data.IntValue < 0 || Data.IntValue >= Planets.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID FROM PLANET INDEX"))
		return {false, "Invalid from planet given."};
	}
	if(Data.IntValue2 < 0 || Data.IntValue2 >= Planets.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID TO PLANET INDEX"))
		return {false, "Invalid to planet given."};
	}
	
	const APlanet* FromPlanet = Planets[Data.IntValue];
	APlanet* ToPlanet = Planets[Data.IntValue2];

	//Ensure player owns from planet
	if(FromPlanet->GetOwningPlayer() != Player->GetPlayerData())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN PLAYER DOES NOT OWN FROM PLANET"))
		return {false, "You do not own the starting planet."};
	}

	//Ensure player DOESN'T own to planet
	if(ToPlanet->GetOwningPlayer() == Player->GetPlayerData())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN PLAYER OWNS TO PLANET"))
		return {false, "You cannot attack your own planets."};
	}

	//Ensure from planet has enough ships
	if(FromPlanet->GetShipAmount() < Data.ShipAmount)
	{
		UE_LOG(LogTemp, Error, TEXT("NOT ENOUGH SHIPS ON FROM PLANET"))
		return {false, "You do not have the inputted amount of ships on the starting planet."};
	}

	//Set attack line
	ToPlanet->AddIncomingAttackLine(Player->GetPlayerData(), FromPlanet->GetPlanetIndex(), Data.ShipAmount);
	return {true, ""};
}
