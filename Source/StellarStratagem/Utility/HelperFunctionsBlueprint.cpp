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

bool UHelperFunctionsBlueprint::IsUsernameAllowed(const FString& Username, FString& OutResult)
{
	if(Username.IsEmpty())
	{
		OutResult = "Username must be set.";
		return false;
	}
	
	if(Username.Len() < 3)
	{
		OutResult = "Username is too short.";
		return false;
	}

	if(Username.Len() > 25)
	{
		OutResult = "Username is too long.";
		return false;
	}

	const TArray<TCHAR> UsernameArr = Username.GetCharArray();
	if(UsernameArr.ContainsByPredicate([](const TCHAR C){ return FChar::IsWhitespace(C); }))
	{
		OutResult = "Username cannot contain white spaces.";
		return false;
	}

	return true;
}
