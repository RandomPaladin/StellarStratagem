#pragma once

#include "CoreMinimal.h"
#include "PlayerData.generated.h"

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Username;
	
	FPlayerData()
	{
		Username = "";
	}

	FPlayerData(FString InUsername)
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
};

FORCEINLINE uint32 GetTypeHash(const FPlayerData& PlayerData)
{
	return GetTypeHash(PlayerData.Username);
}