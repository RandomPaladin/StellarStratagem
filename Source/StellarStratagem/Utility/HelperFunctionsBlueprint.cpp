#include "HelperFunctionsBlueprint.h"

bool UHelperFunctionsBlueprint::Equal_PlayerData(const FPlayerData& A, const FPlayerData& B)
{
	return A == B;
}

bool UHelperFunctionsBlueprint::NotEqual_PlayerData(const FPlayerData& A, const FPlayerData& B)
{
	return A != B;
}

bool UHelperFunctionsBlueprint::IsValid_PlayerData(const FPlayerData& A)
{
	return A.IsValid();
}
