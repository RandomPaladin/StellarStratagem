#include "StellarPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Actions/ActionType.h"
#include "StellarStratagem/Actions/BuildAction.h"
#include "StellarStratagem/Actions/EndTurnAction.h"
#include "StellarStratagem/Actions/ProductionDistributionUpdateAction.h"
#include "StellarStratagem/Actions/SetAttackLineAction.h"
#include "StellarStratagem/Gameplay/GameManager.h"
#include "StellarStratagem/Gameplay/Planet.h"
#include "StellarStratagem/Gameplay/ServerManager.h"
#include "StellarStratagem/Gameplay/ShipAttackLine.h"

void AStellarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStellarPlayerController, PlayerData);
	DOREPLIFETIME(AStellarPlayerController, GoldAmount);
	DOREPLIFETIME(AStellarPlayerController, TechLevel);
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

void AStellarPlayerController::AddGold(const int Gold)
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

void AStellarPlayerController::RemoveGold(const int Gold)
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

void AStellarPlayerController::AddTechXP(const float Xp)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD TECH XP OUTSIDE OF SERVER"))
		return;
	}

	//Add xp
	TechLevel += Xp;
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
				if(InitiallyPressedPlanet->IsOwnedByPlayer(PlayerData) && InitiallyPressedPlanet->GetAvailableShipAmount() >= 1)
				{
					AShipAttackLine* AttackLine = InitiallyPressedPlanet->CreateAttackLine();
					CurrentShipAttackLine = AttackLine;
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
			if(ReleasedOnPlanet && !ReleasedOnPlanet->IsOwnedByPlayer(PlayerData))
			{
				//If an attack line between these planets already exists, use that one instead
				AShipAttackLine* ExistingAttackLine = InitiallyPressedPlanet->GetShipAttackLineToPlanet(ReleasedOnPlanet);
				if(ExistingAttackLine)
				{
					InitiallyPressedPlanet->RemoveAttackLine(CurrentShipAttackLine);
					CurrentShipAttackLine = ExistingAttackLine;
				}
				else //No existing attack line, setup this one
					CurrentShipAttackLine->SetupAttackLine(this, InitiallyPressedPlanet, ReleasedOnPlanet);
				
				OnShipAttackLineCreated.Broadcast(CurrentShipAttackLine);
			}
			else //Destroy attack line if not released on an enemy planet
				InitiallyPressedPlanet->RemoveAttackLine(CurrentShipAttackLine);

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

APlanet* AStellarPlayerController::GetHoveredPlanet(const FVector& ScreenLoc)
{
	const FVector WorldLoc = ScreenToWorldLoc(ScreenLoc);
	for (APlanet* Planet : GetGameManager()->GetPlanets())
	{
		const float SqrDist = FVector::DistSquared(Planet->GetActorLocation(), WorldLoc);
		if(SqrDist > PlanetSelectRadiusSqr)
			continue;

		return Planet;
	}

	return nullptr;
}

APlanet* AStellarPlayerController::GetSelectedPlanet()
{
	//If no planet selected, find one that is owned by self
	if(!SelectedPlanet)
		SelectedPlanet = *GetGameManager()->GetPlanets().FindByPredicate([this](const APlanet* Planet){ return Planet->IsOwnedByPlayer(PlayerData); });

	return SelectedPlanet;
}

#pragma endregion