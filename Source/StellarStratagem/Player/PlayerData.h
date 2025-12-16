#pragma once

#include "CoreMinimal.h"
#include "PlayerData.generated.h"

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Username;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int GoldAmount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TechLevel = 0.f;
	
	FPlayerData()
	{
		Username = "";
	}

	FPlayerData(FString& InUsername)
	{
		Username = InUsername;
	}
	
	bool IsValid() const { return !Username.IsEmpty(); }
	
	bool operator ==(const FPlayerData& Other) const
	{
		return Username == Other.Username;
	}

	bool operator !=(const FPlayerData& Other) const
	{
		return Username != Other.Username;
	}

	int GetTechLevel() const { return FMath::Floor(TechLevel); }
};

FORCEINLINE uint32 GetTypeHash(const FPlayerData& PlayerData)
{
	return GetTypeHash(PlayerData.Username);
}