#include "EndTurnAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"

FActionResult UEndTurnAction::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	//Ensure this is only attempted on the server
	if(!GameManager->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO END TURN OUTSIDE OF SERVER"))
		return {false, "Action was not performed on the server."};
	}
	
	//Ensure player is awaited
	if(!GameManager->GetAwaitedPlayers().ContainsByPredicate([Player](const FPlayerData& PlayerData){ return PlayerData == Player->GetPlayerData(); }))
	{
		UE_LOG(LogTemp, Error, TEXT("PLAYER IS NOT AWAITED"))
		return {false, "You have already ended your turn this round."};
	}

	//End turn
	UE_LOG(LogTemp, Warning, TEXT("PLAYER %s ENDED THEIR TURN"), *Player->GetPlayerData().Username)
	GameManager->EndTurn(Player);
	return {true, ""};
}
