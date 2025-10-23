#include "BuildAction.h"
#include "StellarStratagem/Data/PlanetBuildingsData.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"

FActionResult UBuildAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
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

	const FBuildingSlot Slot = Planet->GetBuildingSlots()[BuildingSlotIndex];
	const TEnumAsByte<EBuildingType> TargetBuildingType = Data.BuildingType;
	
	const bool HasCurrentBuilding = Slot.CurrentBuildingType != BuildingType_None;
	const bool HasTargetBuilding = TargetBuildingType != BuildingType_None;
	
	//Ensure player has enough gold if trying to plan new building
	if(!HasCurrentBuilding && HasTargetBuilding) 
	{
		//Ensure the player has enough gold
		const int GoldCost = Planet->GetBuildingsData()->Buildings[TargetBuildingType].GoldCost;
		if(Player->GetGold() < GoldCost)
		{
			UE_LOG(LogTemp, Error, TEXT("PLAYER DOES NOT HAVE ENOUGH GOLD TO BUILD THIS"))
			return {false, FString::Printf(TEXT("You need %d credits to build this."), GoldCost)};
		}
	}
	
	Planet->UpdateBuilding(Player, BuildingSlotIndex, Data.BuildingType);
	return {true, ""};
}
