#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/ServerManager.h"

void AStellarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStellarPlayerController, Username);
}

void AStellarPlayerController::BeginPlay()
{
	//Get server manager
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));

	//Set random username
	if(HasAuthority())
		Username = FString::FromInt(FMath::RandRange(0, 10000000));

	Super::BeginPlay();
}

void AStellarPlayerController::SendAction_Server_Implementation(const FActionData& ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("RECEIVED DATA %d"), ActionData.NumberTest)
}

void AStellarPlayerController::TryCreateGame_Server_Implementation(const FString& GameCode)
{
	const bool Succeeded = ServerManager->TryCreateGame(this, GameCode);
	if(Succeeded)
		CurrentGameCode = GameCode;
}

void AStellarPlayerController::TryJoinGame_Server_Implementation(const FString& GameCode)
{
	const bool Succeeded = ServerManager->TryJoinGame(this, GameCode);
	if(Succeeded)
		CurrentGameCode = GameCode;
}

void AStellarPlayerController::TryLeaveGame_Server_Implementation()
{
	ServerManager->TryLeaveGame(this);
}

void AStellarPlayerController::TryStartGame_Server_Implementation()
{
	//Get game and ensure game exists
	AGameManager* CurrentGame = ServerManager->GetGame(CurrentGameCode);
	if(!CurrentGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME %s NOT FOUND"), *CurrentGameCode)
		return;
	}

	//Start game
	CurrentGame->StartGame();
}

void AStellarPlayerController::TryEndTurn_Server_Implementation()
{
	//Get game and ensure game exists
	AGameManager* CurrentGame = ServerManager->GetGame(CurrentGameCode);
	if(!CurrentGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAME %s NOT FOUND"), *CurrentGameCode)
		return;
	}

	//End turn
	CurrentGame->EndTurn(this);
}

void AStellarPlayerController::CloseApplication()
{
	//TODO REMOVE SELF FROM GAME LOBBY

	//Close app
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}
