#include "OnlineGameInstance.h"
#include "Online.h"
#include "Kismet/GameplayStatics.h"

void UOnlineGameInstance::EnsureSetup()
{
	OnlineSub = IOnlineSubsystem::Get();
	SessionInterface = OnlineSub->GetSessionInterface();
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnFindSessionsComplete);
	OnDestroyOldSessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOnlineGameInstance::OnDestroyOldSessionComplete);
}

#pragma region Creating Session

void UOnlineGameInstance::CreateSession()
{
	EnsureSetup();

	//Ensure a session doesn't exist already. If it does, destroy it before making a new one
	SessionInterface->DestroySession(TestSessionName, OnDestroyOldSessionCompleteDelegate);
}

void UOnlineGameInstance::OnDestroyOldSessionComplete(FName SessionName, bool Successful)
{
	if (Successful)
		UE_LOG(LogTemp, Warning, TEXT("OLD SESSION DESTROYED"))
	else
		UE_LOG(LogTemp, Warning, TEXT("NO OLD SESSION DESTROYED"))

	//Create session settings
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = false;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings->bIsDedicated = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->NumPublicConnections = 2;

	//Create session
	bool Succeeded = SessionInterface->CreateSession(*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), TestSessionName, *SessionSettings);

	//Handle session creation success
	if (Succeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("SESSION CREATED SUCCESSFULLY"))

		//Go to lobby map
		UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), LobbyMap, true, "listen");
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("FAILED TO CREATE SESSION"))
}

#pragma endregion

#pragma region Finding Sessions

void UOnlineGameInstance::FindSessions()
{
	EnsureSetup();

	//Create search settings
	SessionSearchSettings = MakeShareable(new FOnlineSessionSearch());
	SessionSearchSettings->bIsLanQuery = false;
	SessionSearchSettings->MaxSearchResults = 10;
	SessionSearchSettings->TimeoutInSeconds = 10.f;

	//Bind to on session search complete
	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	
	//Start session search
	bool SearchStarted = SessionInterface->FindSessions(*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), SessionSearchSettings.ToSharedRef());

	if (SearchStarted)
		UE_LOG(LogTemp, Warning, TEXT("SESSION SEARCH STARTED"))
	else
		UE_LOG(LogTemp, Warning, TEXT("SESSION SEARCH FAILED TO START"))
}

void UOnlineGameInstance::OnFindSessionsComplete(bool Successful)
{
	//Unbind from on session search complete
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

	//TODO JOIN A FOUND SESSION

	if(Successful)
		UE_LOG(LogTemp, Warning, TEXT("SESSION SEARCH FOUND %d SESSIONS"), SessionSearchSettings->SearchResults.Num())
	else
		UE_LOG(LogTemp, Warning, TEXT("SESSION SEARCH FAILED"))
}

#pragma endregion