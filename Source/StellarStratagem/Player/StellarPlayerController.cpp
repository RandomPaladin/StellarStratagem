#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Actions/ActionType.h"
#include "StellarStratagem/Actions/BuildAction.h"
#include "StellarStratagem/Actions/CancelAttackLineAction.h"
#include "StellarStratagem/Actions/EndTurnAction.h"
#include "StellarStratagem/Actions/ProductionDistributionUpdateAction.h"
#include "StellarStratagem/Actions/SetAttackLineAction.h"
#include "StellarStratagem/Data/LocalUserSaveGame.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"
#include "StellarStratagem/Gameplay/ShipAttackLine.h"
#include "StellarStratagem/Gameplay/ShipAttackLineManager.h"
#include "StellarStratagem/Multiplayer/OnlineGameInstance.h"

void AStellarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStellarPlayerController, PlayerDataIndex);
}

void AStellarPlayerController::SetPlayerDataIndex(const int NewIndex)
{
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO SET PLAYER DATA INDEX OUTSIDE OF SERVER"))
		return;
	}
	
	PlayerDataIndex = NewIndex;
}

void AStellarPlayerController::SetLocalUsername(FString NewLocalUsername)
{
	LocalUsername = NewLocalUsername;
	UHelperFunctions::SaveLocalUsernameData(LocalUsername);
}

void AStellarPlayerController::BeginPlay()
{
	//Calculate sqr of planet select radius
	PlanetSelectRadiusSqr = FMath::Pow(PlanetSelectRadius, 2.f);

	OnlineGameInstance = Cast<UOnlineGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	if(!UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		//Set local username
		const ULocalUserSaveGame* LocalSaveGame = UHelperFunctions::GetLocalUserData();
		if(LocalSaveGame)
			SetLocalUsername(LocalSaveGame->Username);

		//Send player data to server
		if(GameManager)
		{
			UE_LOG(LogTemp, Log, TEXT("SENDING PLAYER DATA TO SERVER"))
			const FPlayerData PlayerData = {LocalUsername};
			SendPlayerData_Server(PlayerData);
		}
	}
	
	Super::BeginPlay();
}

#pragma region Game Creation / Joining

void AStellarPlayerController::TryCreateGame(const FString& GameCode)
{
	UHelperFunctions::SaveCurrentGameCodeData(GameCode);
	UHelperFunctions::SaveCurrentIsGameCreatorData(true);
	UE_LOG(LogTemp, Warning, TEXT("SETTING PLAYERS GAME CODE TO %s"), *GameCode)

	OnlineGameInstance->CreateGame(this, GameCode);
}

void AStellarPlayerController::TryJoinGame(const FString& GameCode)
{
	UHelperFunctions::SaveCurrentGameCodeData(GameCode);
	UHelperFunctions::SaveCurrentIsGameCreatorData(false);
	UE_LOG(LogTemp, Warning, TEXT("SETTING PLAYERS GAME CODE TO %s"), *GameCode)

	OnlineGameInstance->JoinGame(this, GameCode);
}

void AStellarPlayerController::TryStartGame_Server_Implementation()
{
	GameManager->StartGame();
}

#pragma endregion

void AStellarPlayerController::SendPlayerData_Server_Implementation(const FPlayerData& PlayerData)
{
	GameManager->ReceivePlayerDataFromClient(this, PlayerData);
}

void AStellarPlayerController::OnRep_PlayerDataIndex()
{
	GameManager->OnPlayersUpdated.Broadcast();
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

FPlayerData AStellarPlayerController::GetPlayerData() const
{
	if(PlayerDataIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO GET PLAYER DATA BUT NO INDEX IS SET YET"))
		FDebug::DumpStackTraceToLog(ELogVerbosity::Type::Warning);
		return {};
	}
	
	return GameManager->GetPlayerDataByIndex(PlayerDataIndex);
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
		case ActionType_ProductionDistributionUpdate:
			ClassType = UProductionDistributionUpdateAction::StaticClass();
			break;
		case ActionType_SetAttackLine:
			ClassType = USetAttackLineAction::StaticClass();
			break;
		case ActionType_CancelAttackLine:
			ClassType = UCancelAttackLineAction::StaticClass();
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("ACTION TYPE IS NOT BEING HANDLED"))
			return;
	}

	//Create and perform action
	UActionBase* Action = NewObject<UActionBase>(GetTransientPackage(), ClassType);
	Action->Data = ActionData;
	const FActionResult Result = Action->PerformAction(GameManager, this);

	//Send result to client
	ReceiveActionResult_Client(Result);
}

void AStellarPlayerController::ReceiveActionResult_Client_Implementation(const FActionResult& ActionResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ACTION RESULT: %s %s"), *((ActionResult.Succeeded) ? FString{"true"} : FString{"false"}), *ActionResult.Message)
	
	if(!ActionResult.Succeeded)
		ShowMessage(ActionResult.Message);
}

#pragma endregion

#pragma region Input

void AStellarPlayerController::OnPress(const FVector& Loc)
{
	if(!IsInputOccluded())
		return;
	
	//Initial values
	StartTouchLoc = Loc;
	PreviousTouchLoc = StartTouchLoc;
	CurrentTouchLoc = StartTouchLoc;
	DraggingFromPlanet = false;
	CurrentShipAttackLine = nullptr;
	InitiallyPressedPlanet = GetHoveredPlanet(Loc);
}

void AStellarPlayerController::OnPressMoved(const FVector& Loc)
{
	if(!IsInputOccluded())
		return;

	if(!CamActor)
		return;
	
	//Record current touch loc
	CurrentTouchLoc = Loc;

	//Ignore if two fingers touching
	if(TwoFingersTouching)
		return;
	
	//Handle dragging out of planet
	if(InitiallyPressedPlanet)
	{
		//Get world locs
		const FVector PlanetLoc = InitiallyPressedPlanet->GetActorLocation();
		const FVector TouchWorldLoc = ScreenToWorldLoc(Loc);

		//Check if dragged away from owned planet
		if(!DraggingFromPlanet)
		{
			const float SqrDist = FVector::DistSquared(PlanetLoc, TouchWorldLoc);
			if(SqrDist > PlanetSelectRadiusSqr)
			{
				DraggingFromPlanet = true;
				
				//Create attack line if dragging from owned planet with enough ships
				if(InitiallyPressedPlanet->IsOwnedByPlayer(GetPlayerData()) && InitiallyPressedPlanet->GetAvailableShipAmount() >= 1)
				{
					CurrentShipAttackLine = GetShipAttackLineManager()->CreateAttackLine();
					CurrentShipAttackLine->SetFromLoc(InitiallyPressedPlanet->GetActorLocation());
				}
			}
		}
		
		//Set attack line target loc
		if(DraggingFromPlanet && CurrentShipAttackLine)
			CurrentShipAttackLine->SetTargetLoc(TouchWorldLoc);
	}
	else //Update camera loc if not dragging from a planet
		CamActor->SetActorLocation(CamActor->GetActorLocation() + ScreenToWorldDelta(PreviousTouchLoc - CurrentTouchLoc));

	//Record previous touch loc
	PreviousTouchLoc = CurrentTouchLoc;
}

void AStellarPlayerController::OnPressReleased(const FVector& Loc)
{
	//Allow press release to happen even if input is occluded
	
	//No initial planet
	if(!InitiallyPressedPlanet)
		return;

	//Update ship attack target if dragged from owned planet
	if(DraggingFromPlanet)
	{
		//Setup attack line
		if(CurrentShipAttackLine)
		{
			APlanet* ReleasedOnPlanet = GetHoveredPlanet(Loc);
			if(ReleasedOnPlanet && !ReleasedOnPlanet->IsOwnedByPlayer(GetPlayerData()))
			{
				//If an attack line between these planets already exists, use that one instead
				AShipAttackLine* ExistingAttackLine = GetShipAttackLineManager()->GetShipAttackLineToPlanet(ReleasedOnPlanet);
				if(ExistingAttackLine)
				{
					GetShipAttackLineManager()->RemoveAttackLine(CurrentShipAttackLine);
					CurrentShipAttackLine = ExistingAttackLine;
				}
				else //No existing attack line, setup this one
					CurrentShipAttackLine->SetupAttackLine(this, InitiallyPressedPlanet, ReleasedOnPlanet, 1);
				
				OnShipAttackLineCreated.Broadcast(CurrentShipAttackLine);
			}
			else //Destroy attack line if not released on an enemy planet
				GetShipAttackLineManager()->RemoveAttackLine(CurrentShipAttackLine);

			CurrentShipAttackLine = nullptr;
		}
	}
	else //Select planet if didn't drag from it
	{
		//Select planet
		SelectedPlanet = InitiallyPressedPlanet;
		OnPlanetSelected.Broadcast(SelectedPlanet);
	}
}

void AStellarPlayerController::UpdateInputOcclusion(UObject* Occluder, const bool Occluding)
{
	if(Occluding)
		InputOccluders.AddUnique(Occluder);
	else
		InputOccluders.Remove(Occluder);
}

#pragma endregion

#pragma region Helpers

AShipAttackLineManager* AStellarPlayerController::GetShipAttackLineManager()
{
	if(!ShipAttackLineManager)
		ShipAttackLineManager = Cast<AShipAttackLineManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AShipAttackLineManager::StaticClass()));

	return ShipAttackLineManager;
}

FVector AStellarPlayerController::ScreenToWorldLoc(const FVector& ScreenLoc) const
{
	FVector WorldLoc;
	FVector WorldDir;
	DeprojectScreenPositionToWorld(ScreenLoc.X, ScreenLoc.Y, WorldLoc, WorldDir);
	const float DistanceToZPlane = -(WorldLoc.Z / WorldDir.Z); //Shortened from a^2 + b^2 = c^2
	const FVector WorldLocOnZPlane = WorldLoc + WorldDir * DistanceToZPlane;

	return WorldLocOnZPlane;
}

FVector AStellarPlayerController::ScreenToWorldDelta(const FVector& ScreenDelta) const
{
	const FVector StartWorldLoc = ScreenToWorldLoc({0, 0, 0});
	const FVector EndWorldLoc = ScreenToWorldLoc(ScreenDelta);
	return EndWorldLoc - StartWorldLoc;
}

APlanet* AStellarPlayerController::GetHoveredPlanet(const FVector& ScreenLoc) const
{
	const FVector WorldLoc = ScreenToWorldLoc(ScreenLoc);
	for (APlanet* Planet : GameManager->GetPlanets())
	{
		const float SqrDist = FVector::DistSquared(Planet->GetActorLocation(), WorldLoc);
		if(SqrDist > PlanetSelectRadiusSqr)
			continue;

		return Planet;
	}

	return nullptr;
}

void AStellarPlayerController::ShowMessage(const FString Message) const
{
	if(Message.IsEmpty())
		return;
	
	OnMessageReceived.Broadcast(Message);
}

APlanet* AStellarPlayerController::GetSelectedPlanet()
{
	//If no planet selected, find one that is owned by self
	if(!SelectedPlanet)
		SelectedPlanet = *GameManager->GetPlanets().FindByPredicate([this](const APlanet* Planet){ return Planet->IsOwnedByPlayer(GetPlayerData()); });

	return SelectedPlanet;
}

#pragma endregion