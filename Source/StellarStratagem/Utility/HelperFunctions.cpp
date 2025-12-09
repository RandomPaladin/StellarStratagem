#include "HelperFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Data/LocalUserSaveGame.h"

void UHelperFunctions::SaveLocalUserData(const FString& Username)
{
	ULocalUserSaveGame* Save = Cast<ULocalUserSaveGame>(UGameplayStatics::CreateSaveGameObject(ULocalUserSaveGame::StaticClass()));

	Save->Username = Username;

	UGameplayStatics::SaveGameToSlot(Save, TEXT("LocalUserData"), 0);
}

ULocalUserSaveGame* UHelperFunctions::GetLocalUserData()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("LocalUserData"), 0))
		return Cast<ULocalUserSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LocalUserData"), 0));

	return nullptr;
}
