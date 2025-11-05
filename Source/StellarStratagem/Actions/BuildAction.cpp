#include "BuildAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult UBuildAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	//Ensure this is only done on the server
	if(!GameManager->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO BUILD ON PLANET OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}
	
	//Ensure given planet index is valid
	if(Data.IntValue < 0 || Data.IntValue >= GameManager->GetPlanets().Num())
	{
		UE_LOG(LogTemp, Error, TEXT("INVALID PLANET INDEX"))
		return {false, "Invalid planet given."};
	}
	
	APlanet* Planet = GameManager->GetPlanets()[Data.IntValue];
	const int BuildingSlotIndex = Data.IntValue2;

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

	const FBuildingSlot Slot = Planet->GetBuildingSlots()[BuildingSlotIndex];
	const TEnumAsByte<EBuildingType> TargetBuildingType = Data.BuildingType;
	
	const bool HasCurrentBuilding = Slot.CurrentBuildingType != BuildingType_None;
	const bool HasTargetBuilding = TargetBuildingType != BuildingType_None;
	
	//Ensure player has enough gold if trying to plan new building
	if(!HasCurrentBuilding && HasTargetBuilding) 
	{
		//Ensure the player has enough gold
		const int GoldCost = Planet->GetPlanetData()->Buildings[TargetBuildingType].GoldCost;
		if(Player->GetGold() < GoldCost)
		{
			UE_LOG(LogTemp, Error, TEXT("PLAYER DOES NOT HAVE ENOUGH GOLD TO BUILD THIS"))
			return {false, FString::Printf(TEXT("You need %d credits to build this."), GoldCost)};
		}
	}

	//Update building
	Planet->UpdateBuilding(Player, BuildingSlotIndex, Data.BuildingType);
	return {true, ""};
}
