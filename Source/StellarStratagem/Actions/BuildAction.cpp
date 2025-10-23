#include "BuildAction.h"
#include "StellarStratagem/Data/PlanetBuildingsData.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult UBuildAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	Super::PerformAction(GameManager, Player);

	APlanet* Planet = GameManager->GetPlanets()[Data.Number];
	const int BuildingSlotIndex = Data.Number2;

	//Ensure this is only done on the server
	if(!Planet->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO BUILD ON PLANET OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}

	//Ensure player owns this planet
	if(Planet->GetOwningPlayer() != Player->GetPlayerData())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN PLAYER DOES NOT OWN THIS PLANET"))
		return {false, "You do not own this planet."};
	}

	//Ensure given building slot is valid
	if(BuildingSlotIndex < 0 || BuildingSlotIndex >= Planet->GetBuildingSlots().Num())
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN BUILDING SLOT INDEX IS OUTSIDE OF SLOT ARRAY"))
		return {false, "Invalid building slot index."};
	}

	//Ensure given building type is valid
	if(Data.BuildingType == BuildingType_None)
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN BUILDING TYPE IS NOT VALID"))
		return {false, "Given building type is invalid."};
	}

	//Ensure given building slot is empty
	if(Planet->GetBuildingSlots()[BuildingSlotIndex].BuildingType != BuildingType_None)
	{
		UE_LOG(LogTemp, Error, TEXT("GIVEN BUILDING SLOT IS ALREADY FILLED"))
		return {false, "This building slot is already in use."};
	}

	//Ensure player has enough gold to build building
	const int GoldCost = Planet->GetBuildingsData()->Buildings[Data.BuildingType].GoldCost;
	if(Player->GetGold() < GoldCost)
	{
		UE_LOG(LogTemp, Error, TEXT("PLAYER DOES NOT HAVE ENOUGH GOLD TO BUILD THIS"))
		return {false, FString::Printf(TEXT("You need %d credits to build this."), GoldCost)};
	}
	
	Planet->Build(Player, BuildingSlotIndex, Data.BuildingType);
	return {true, ""};
}
