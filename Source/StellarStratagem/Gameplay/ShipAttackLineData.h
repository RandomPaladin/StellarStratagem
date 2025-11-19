#pragma once

#include "CoreMinimal.h"
#include "StellarStratagem/Player/PlayerData.h"
#include "ShipAttackLineData.generated.h"

class AGameManager;

USTRUCT(BlueprintType)
struct FShipAttackLineData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FPlayerData Player;
	UPROPERTY(VisibleAnywhere)
	int FromPlanetIndex;
	UPROPERTY(VisibleAnywhere)
	int ShipAmount;
	UPROPERTY(VisibleAnywhere)
	float Progress;

	bool IsValid(const AGameManager* GameManager) const;
	
	FShipAttackLineData()
	{
		Player = {};
		FromPlanetIndex = 0;
		ShipAmount = 0;
		Progress = 0.f;
	}

	FShipAttackLineData(const FPlayerData& InPlayer, const int InFromPlanetIndex, const int InShipAmount)
	{
		Player = InPlayer;
		FromPlanetIndex = InFromPlanetIndex;
		ShipAmount= InShipAmount;
		Progress = 0.f;
	}

	bool operator ==(const FShipAttackLineData& Other) const
	{
		return Player == Other.Player && FromPlanetIndex == Other.FromPlanetIndex;
	}
};