#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Actions/ActionType.h"
#include "StellarStratagem/Actions/BuildAction.h"
#include "StellarStratagem/Actions/EndTurnAction.h"
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

	//On server setup
	if(HasAuthority())
	{
		//Set random username
		PlayerData = {FString::FromInt(FMath::RandRange(0, 10000000))};

		//Set initial gold amount
		GoldAmount = StartingGold;
	}

	Super::BeginPlay();
}

#pragma region Game Creation / Joining

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
	GetGameManager()->StartGame();
}

#pragma endregion

#pragma region Replication Funcs

void AStellarPlayerController::OnRep_GoldAmount() const
{
	OnGoldUpdated.Broadcast(GoldAmount);
}

#pragma endregion

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

void AStellarPlayerController::RemoveGold(int Gold)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD GOLD OUTSIDE OF SERVER"))
		return;
	}

	//Remove gold, don't go under 0
	GoldAmount = FMath::Max(GoldAmount - Gold, 0);
}

#pragma endregion

#pragma region Actions

void AStellarPlayerController::SendAction_Server_Implementation(const FActionData& ActionData)
{
	//Find action's class type
	UClass* ClassType;
	switch (ActionData.ActionType)
	{
		case ActionType_None:
			UE_LOG(LogTemp, Error, TEXT("GIVEN ACTION HAS NO TYPE"))
			return;
		case ActionType_Build:
			ClassType = UBuildAction::StaticClass();
			break;
		case ActionType_EndTurn:
			ClassType = UEndTurnAction::StaticClass();
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("ACTION TYPE IS NOT BEING HANDLED"))
			return;
	}

	//Create and perform action
	UActionBase* Action = NewObject<UActionBase>(GetTransientPackage(), ClassType);
	Action->Data = ActionData;
	const FActionResult Result = Action->PerformAction(GetGameManager(), this);

	//Send result to client
	ReceiveActionResult_Client(Result);
}

void AStellarPlayerController::ReceiveActionResult_Client_Implementation(const FActionResult& ActionResult)
{
	if(ActionResult.Message.IsEmpty())
		return;
	
	UE_LOG(LogTemp, Warning, TEXT("RESULT: %s"), *ActionResult.Message)
	OnMessageReceived.Broadcast(ActionResult.Message);
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
	
	//Find world loc
	FVector WorldLoc = ScreenToWorldLoc(Loc);

	//Find a planet that's close enough to select
	APlanet* PlanetToSelect = nullptr;
	for (APlanet* Planet : GetGameManager()->GetPlanets())
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

AGameManager* AStellarPlayerController::GetGameManager()
{
	if(!GameManager)
	{
		if(HasAuthority())
			GameManager = ServerManager->GetGame(CurrentGameCode);
		else
			GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	}

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
		SelectedPlanet = *GetGameManager()->GetPlanets().FindByPredicate([this](const APlanet* Planet){ return Planet->IsOwnedByPlayer(PlayerData); });

	return SelectedPlanet;
}

#pragma endregion