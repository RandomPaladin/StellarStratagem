#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Actions/ActionBase.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"
#include "StellarStratagem/Gameplay/ServerManager.h"

void AStellarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStellarPlayerController, PlayerData);
	DOREPLIFETIME(AStellarPlayerController, GoldAmount);
}

void AStellarPlayerController::BeginPlay()
{
	//Calculate sqr of planet select radius
	PlanetSelectRadiusSqr = FMath::Pow(PlanetSelectRadius, 2.f);
	
	//Get server manager
	ServerManager = Cast<AServerManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AServerManager::StaticClass()));

	//Set random username
	if(HasAuthority())
		PlayerData = {FString::FromInt(FMath::RandRange(0, 10000000))};

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
		UE_LOG(LogTemp, Error, TEXT("GAME %s NOT FOUND"), *CurrentGameCode)
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

void AStellarPlayerController::GoToMainMenu()
{
	//TODO EXIT SAFELY

	//Go back to main menu
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainGameMap);
}

#pragma region Gameplay

void AStellarPlayerController::AddGold(int Gold)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD GOLD OUTSIDE OF SERVER"))
		return;
	}

	//Add gold
	GoldAmount += Gold;
}

#pragma endregion

#pragma region Input

void AStellarPlayerController::OnPress(const FVector& Loc)
{
	StartTouchLoc = Loc;
	StartCamLoc = CamActor->GetActorLocation();
}

void AStellarPlayerController::OnPressMoved(const FVector& Loc)
{
	//Record current touch loc
	CurrentTouchLoc = Loc;

	//Ignore if two fingers touching
	if(TwoFingersTouching)
		return;

	//Update camera loc
	FVector Offset = (CurrentTouchLoc - StartTouchLoc) * ScrollAcceleration;
	CamActor->SetActorLocation(StartCamLoc + Offset);
}

void AStellarPlayerController::OnPressReleased(const FVector& Loc)
{
	//TODO ONLY SELECT IF DIDN'T DRAG OUT OF PLANET
	
	//Get game
	AGameManager* Game = GetGameOnClient();

	//Find world loc
	FVector WorldLoc = ScreenToWorldLoc(Loc);

	//Find a planet that's close enough to select
	APlanet* PlanetToSelect = nullptr;
	for (APlanet* Planet : Game->GetPlanets())
	{
		float SqrDist = FVector::DistSquared(Planet->GetActorLocation(), WorldLoc);
		if(SqrDist > PlanetSelectRadiusSqr)
			continue;

		PlanetToSelect = Planet;
		break;
	}

	//No selectable planet found, return
	if(!PlanetToSelect)
		return;

	//Select planet
	SelectedPlanet = PlanetToSelect;
	OnPlanetSelected.Broadcast(PlanetToSelect);
}

#pragma endregion

#pragma region Helpers

AGameManager* AStellarPlayerController::GetGameOnClient()
{
	if(!GameManager)
		GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	return GameManager;
}

FVector AStellarPlayerController::ScreenToWorldLoc(const FVector& ScreenLoc) const
{
	FVector WorldLoc;
	FVector WorldDir;
	DeprojectScreenPositionToWorld(ScreenLoc.X, ScreenLoc.Y, WorldLoc, WorldDir);
	const float DistanceToZPlane = -(WorldLoc.Z / WorldDir.Z);
	const FVector WorldLocOnZPlane = WorldLoc + WorldDir * DistanceToZPlane;

	return WorldLocOnZPlane;
}

APlanet* AStellarPlayerController::GetSelectedPlanet()
{
	//If no planet selected, find one that is owned by self
	if(!SelectedPlanet)
		SelectedPlanet = *GetGameOnClient()->GetPlanets().FindByPredicate([this](APlanet* Planet){ return Planet->IsOwnedByPlayer(PlayerData); });

	return SelectedPlanet;
}

#pragma endregion