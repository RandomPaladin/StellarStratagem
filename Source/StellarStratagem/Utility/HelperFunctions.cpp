#include "HelperFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Data/LocalUserSaveGame.h"

ULocalUserSaveGame* UHelperFunctions::GetExistingOrNewLocalUserData()
{
	ULocalUserSaveGame* Save = GetLocalUserData();
	if(Save)
		return Save;
	
	return Cast<ULocalUserSaveGame>(UGameplayStatics::CreateSaveGameObject(ULocalUserSaveGame::StaticClass()));
}

void UHelperFunctions::SaveLocalUsernameData(const FString& Username)
{
	ULocalUserSaveGame* Save = GetExistingOrNewLocalUserData();
	Save->Username = Username;

	UGameplayStatics::SaveGameToSlot(Save, TEXT("LocalUserData"), 0);
}

void UHelperFunctions::SaveCurrentGameCodeData(const FString& GameCode)
{
	ULocalUserSaveGame* Save = GetExistingOrNewLocalUserData();
	Save->CurrentGameCode = GameCode;

	UGameplayStatics::SaveGameToSlot(Save, TEXT("LocalUserData"), 0);
}

void UHelperFunctions::SaveCurrentIsGameCreatorData(const bool IsCreator)
{
	ULocalUserSaveGame* Save = GetExistingOrNewLocalUserData();
	Save->IsCurrentGameCreator = IsCreator;

	UGameplayStatics::SaveGameToSlot(Save, TEXT("LocalUserData"), 0);
}

ULocalUserSaveGame* UHelperFunctions::GetLocalUserData()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("LocalUserData"), 0))
		return Cast<ULocalUserSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LocalUserData"), 0));

	return nullptr;
}