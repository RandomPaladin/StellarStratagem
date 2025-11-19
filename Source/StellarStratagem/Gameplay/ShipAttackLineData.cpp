#include "ShipAttackLineData.h"
#include "GameManager.h"

bool FShipAttackLineData::IsValid(const AGameManager* GameManager) const
{
	return Player.IsValid() && FromPlanetIndex >= 0 && FromPlanetIndex < GameManager->GetPlanets().Num();
}
