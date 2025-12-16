#include "GameManager.h"
#include "Planet.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

#pragma region Setup / Lobby

AGameManager::AGameManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGameManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameManager, GameCode);
	DOREPLIFETIME(AGameManager, Round);
	DOREPLIFETIME(AGameManager, GameStarted);
	DOREPLIFETIME(AGameManager, AllPlayers);
	DOREPLIFETIME(AGameManager, PlayersResolutionResults);
}

void AGameManager::BeginPlay()
{
	Super::BeginPlay();
	
	OnPlayersUpdated.Broadcast();
}

void AGameManager::SetupGame(FString NewGameCode)
{
	GameCode = NewGameCode;
}

void AGameManager::ReceivePlayerDataFromClient(AStellarPlayerController* Player, const FPlayerData& PlayerData)
{
	UE_LOG(LogTemp, Log, TEXT("RECEIVED PLAYER DATA IN GAME MANAGER"))
	
	//Add player
	const FPlayerData NewPlayerData = {PlayerData.Username};
	AllPlayers.AddUnique(NewPlayerData);
	AActor* PlayerActor = Player;
	ConnectedPlayers.Add(PlayerActor, Player);
	Player->SetPlayerDataIndex(AllPlayers.Num() - 1);

	ForceNetUpdate();
	
	UE_LOG(LogTemp, Log, TEXT("ADDED PLAYER %s TO GAME"), *NewPlayerData.Username)
}

void AGameManager::RemovePlayer(AStellarPlayerController* Player)
{
	//Ensure adding player is only attempted on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD PLAYER TO GAME OUTSIDE OF SERVER"))
		return;
	}
	
	//Remove player
	AllPlayers.RemoveAll([Player](const FPlayerData& PlayerData) { return PlayerData == Player->GetPlayerData(); });
	AActor* PlayerActor = Player;
	ConnectedPlayers.Remove(PlayerActor);

	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("REMOVED PLAYER %s FROM GAME"), *Player->GetPlayerData().Username)
}

#pragma endregion

#pragma region Gameplay

void AGameManager::StartGame()
{
	//Ensure game start is done on server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO START GAME OUTSIDE OF SERVER"))
		return;
	}

	//Ensure game isn't already started
	if(GameStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("GAME IS ALREADY STARTED"))
		return;
	}

	//Start game
	AwaitedPlayers = AllPlayers;
	GameStarted = true;

	//Create planets
	const int PlanetAmount = GetPlayerAmount() * PlanetData->SpawnPlanetsPerPlayer;
	for (int i = 0; i < PlanetAmount; i++)
	{
		FVector SpawnLoc;
		if(i == 0)
			SpawnLoc = FVector::ZeroVector;
		else
		{
			FVector NewLoc;
			bool IntersectingExistingPlanet;
			int Attempts = 0;
			do
			{
				const float Distance = FMath::RandRange((float)PlanetData->DistanceBetweenPlanetsRange.X, (float)PlanetData->DistanceBetweenPlanetsRange.Y);
				const float UnrealUnitsDistance = Distance * PlanetData->DistanceBetweenPlanetsToUnrealUnitsMultiplier;
				const FVector Dir = FRotator::MakeFromEuler({0.f, 0.f, FMath::RandRange(0.f, 359.f)}).Vector();
				NewLoc = Planets[i - 1]->GetActorLocation() + (Dir * UnrealUnitsDistance);
				
				IntersectingExistingPlanet = UHelperFunctions::Any(Planets, [this, NewLoc](const APlanet* Planet){ return Planet->GetDistanceToLocInGameUnits(NewLoc) < PlanetData->DistanceBetweenPlanetsRange.X; });
				Attempts++;
			}
			while (IntersectingExistingPlanet && Attempts < 100);
			
			SpawnLoc = NewLoc;
		}
		
		FRotator SpawnRot = {0.f, FMath::RandRange(SpawnPlanetRotRange.X, SpawnPlanetRotRange.Y), 0.f};
		APlanet* SpawnedPlanet = GetWorld()->SpawnActor<APlanet>(PlanetTemplate, SpawnLoc, SpawnRot);
		SpawnedPlanet->Setup(this, i);
		Planets.Add(SpawnedPlanet);
		OnPlanetListUpdated.Broadcast();
	}
	
	//Grant a starting planet to each player
	const UEnum* GradeEnum = StaticEnum<EPlanetGrade>();
	const int GradeEnumMiddleIndex = (GradeEnum->NumEnums() - 1) / 2;
	for (const FPlayerData& Player : AllPlayers)
	{
		//Find a suitable planet to grant to player (one closest to the middle grade)
		int StartPlanetIndex = -1;
		int BestDist = 10000;
		for(int i = 0; i < Planets.Num(); i++)
		{
			//Ignore already owned planets
			if(Planets[i]->IsOwnedByAnyPlayer())
				continue;

			//Check if this planet's grade is closer to the middle grade
			const int GradeEnumIndex = GradeEnum->GetIndexByValue(Planets[i]->GetGrade());
			const int NewDist = FMath::Abs(GradeEnumMiddleIndex - GradeEnumIndex);
			if(NewDist < BestDist)
			{
				BestDist = NewDist;
				StartPlanetIndex = i;
			}
		}

		//Ensure a starting planet was found
		if(StartPlanetIndex == -1)
		{
			UE_LOG(LogTemp, Error, TEXT("COULDN'T FIND SUITABLE START PLANET FOR PLAYER"))
			continue;
		}
		
		//Grant starting planet to player
		Planets[StartPlanetIndex]->SetOwningPlayer(Player);
	}

	//Grant starting gold
	for (const FPlayerData& Player : AllPlayers)
		AddGold(Player, StartingGold);
}

void AGameManager::EndTurn(AStellarPlayerController* Player)
{
	//Remove awaited player from list
	AwaitedPlayers.RemoveAll([Player](const FPlayerData& PlayerData){ return PlayerData == Player->GetPlayerData(); });

	//Move on to the next round if all awaited players took their turn
	if(AwaitedPlayers.Num() == 0)
		GoToNextRound();
}

void AGameManager::GoToNextRound()
{
	UE_LOG(LogTemp, Log, TEXT("ALL PLAYERS ENDED THEIR TURN, GOING TO NEXT ROUND"))
	
	//Increment round
	Round++;
	AwaitedPlayers = AllPlayers;

	//Ensure resolution results contains an entry for each player
	for (FPlayerData Player : AllPlayers)
	{
		if(!PlayersResolutionResults.ContainsByPredicate([Player](const FRoundResolutionResults& Results){ return Results.Player == Player; }))
			PlayersResolutionResults.Add(Player);
	}
	
	//Create new resolution results
	TArray<FRoundResolutionResults> NewResults;
	for(int i = 0; i < PlayersResolutionResults.Num(); i++)
	{
		FRoundResolutionResults Result = {PlayersResolutionResults[i].Player};
		NewResults.Add(Result);
	}

	//Generate building resources
	UE_LOG(LogTemp, Log, TEXT("GENERATING BUILDING RESOURCES"))
	TMap<FPlayerData, int> GeneratedGold;
	TMap<FPlayerData, float> GeneratedTechXP;
	for (APlanet* Planet : Planets)
	{
		//Ignore unowned planets
		if(!Planet->IsOwnedByAnyPlayer())
			continue;

		//Ignore newly overtaken planets
		if(Planet->GetNewlyOvertaken())
		{
			Planet->SetNewlyOvertaken(false);
			continue;
		}

		FPlayerData OwningPlayer = Planet->GetOwningPlayer();
		
		//Generate gold
		const int GoldAmount = Planet->GetGeneratedGoldAmount();
		AddGold(OwningPlayer, GoldAmount);

		//Record amount of gold generated per player
		if(GeneratedGold.Contains(OwningPlayer))
			GeneratedGold[OwningPlayer] += GoldAmount;
		else
			GeneratedGold.Add(OwningPlayer, GoldAmount);

		//Generate tech xp
		const float TechXPAmount = Planet->GetGeneratedTechXPAmount();
		AddTechXP(OwningPlayer, TechXPAmount);

		//Record amount of tech xp generated per player
		if(GeneratedTechXP.Contains(OwningPlayer))
			GeneratedTechXP[OwningPlayer] += TechXPAmount;
		else
			GeneratedTechXP.Add(OwningPlayer, TechXPAmount);

		//Generate ships
		const float ShipsGenerated = Planet->GenerateShips();
		
		//Add entries for ship generation
		if(ShipsGenerated > 0.f)
		{
			const int OwningPlayerEntryIndex = GetIndexOfPlayersResolutionResults(Planet->GetOwningPlayer());
			NewResults[OwningPlayerEntryIndex].Results.Add({RoundResolutionResultType_Resources, FString::Printf(TEXT("%.1f ships were produced on %s."), ShipsGenerated, *Planet->GetPlanetName())});
		}
	}

	//Add entries for gold generation
	for (TTuple<FPlayerData, int> Kvp : GeneratedGold)
	{
		//Ignore if no gold was generated
		if(Kvp.Value <= 0)
			continue;
		
		const int OwningPlayerEntryIndex = GetIndexOfPlayersResolutionResults(Kvp.Key);
		NewResults[OwningPlayerEntryIndex].Results.Add({RoundResolutionResultType_Resources, FString::Printf(TEXT("Your planets produced %d credits."), Kvp.Value)});
	}

	//Add entries for tech xp generation
	for (TTuple<FPlayerData, float> Kvp : GeneratedTechXP)
	{
		//Ignore if no tech xp was generated
		if(Kvp.Value <= 0)
			continue;
		
		const int OwningPlayerEntryIndex = GetIndexOfPlayersResolutionResults(Kvp.Key);
		NewResults[OwningPlayerEntryIndex].Results.Add({RoundResolutionResultType_Resources, FString::Printf(TEXT("Your planets produced %.2f tech XP."), Kvp.Value)});
	}
	
	//Resolve building plans
	UE_LOG(LogTemp, Log, TEXT("RESOLVING BUILDING PLANS"))
	for (APlanet* Planet : Planets)
	{
		//Ignore unowned planets
		if(!Planet->IsOwnedByAnyPlayer())
			continue;

		//Resolve
		TArray<TTuple<bool, EBuildingType>> ResolveBuildingPlansResults;
		Planet->ResolveBuildingPlans(ResolveBuildingPlansResults);

		//Add entry to results
		const int OwningPlayerEntryIndex = GetIndexOfPlayersResolutionResults(Planet->GetOwningPlayer());
		for (const TTuple<bool, EBuildingType>& Result : ResolveBuildingPlansResults)
		{
			FString BuildingTypeString;
			switch (Result.Value)
			{
				case BuildingType_None:
					BuildingTypeString = "UNDEFINED";
					break;
				case BuildingType_Factory:
					BuildingTypeString = "Factory";
					break;
				case BuildingType_Research:
					BuildingTypeString = "Research building";
					break;
			}
			FString BuildingResultString = Result.Key ? "built" : "destroyed";
			FString ResultString = FString::Printf(TEXT("%s was %s on planet %s."), *BuildingTypeString, *BuildingResultString, *Planet->GetPlanetName());
			NewResults[OwningPlayerEntryIndex].Results.Add({RoundResolutionResultType_Resources, ResultString});
		}
	}

	//Resolve ship movement
	UE_LOG(LogTemp, Log, TEXT("RESOLVING SHIP MOVEMENT"))
	for (APlanet* Planet : Planets)
	{
		Planet->ResolveShipMovement();
		TArray<FShipAttackLineData> IncomingAttackLines = Planet->GetIncomingAttackLines();
		for (FShipAttackLineData IncomingAttackLine : IncomingAttackLines)
		{
			const int OwningPlayerIndex = GetIndexOfPlayersResolutionResults(IncomingAttackLine.Player);
			FString ResultString = "";
			if(FMath::IsNearlyEqual(IncomingAttackLine.Progress, 1.f))
				ResultString = FString::Printf(TEXT("%d ships have reached planet %s."), IncomingAttackLine.ShipAmount, *Planet->GetPlanetName());
			else
			{
				int Progress = FMath::RoundToInt(IncomingAttackLine.Progress * 100.f);
				ResultString = FString::Printf(TEXT("%d ships have traveled %d%% of the way from planet %s to planet %s."), IncomingAttackLine.ShipAmount, Progress, *Planets[IncomingAttackLine.FromPlanetIndex]->GetPlanetName(), *Planet->GetPlanetName());
			}
			
			NewResults[OwningPlayerIndex].Results.Add({RoundResolutionResultType_Resources, ResultString});
		}
	}

	//Resolve combat
	UE_LOG(LogTemp, Log, TEXT("RESOLVING COMBAT"))
	for (APlanet* Planet : Planets)
	{
		//Get attack lines
		TArray<FShipAttackLineData> IncomingAttackLines = Planet->GetIncomingAttackLines();

		//Sort attack lines by tech level
		IncomingAttackLines.Sort([this](const FShipAttackLineData& AttackLine1, const FShipAttackLineData& AttackLine2)
		{
			const int Player1Index = GetPlayerDataIndex(AttackLine1.Player);
			const int Player2Index = GetPlayerDataIndex(AttackLine2.Player);

			const FPlayerData& Player1Data = AllPlayers[Player1Index];
			const FPlayerData& Player2Data = AllPlayers[Player2Index];
			
			return Player1Data.GetTechLevel() > Player2Data.GetTechLevel();
		});
	
		for (int i = 0; i < IncomingAttackLines.Num(); i++)
		{
			//Ignore attack line if ships haven't reached planet yet
			if(!FMath::IsNearlyEqual(IncomingAttackLines[i].Progress, 1.f))
				continue;

			FRoundResolutionResult CombatResolutionResult;
			//Targeted planet isn't owned by anyone or is owned by player, just grant it straight up 
			if(!Planet->IsOwnedByAnyPlayer() || Planet->IsOwnedByPlayer(IncomingAttackLines[i].Player))
			{
				Planet->SetOwningPlayer(IncomingAttackLines[i].Player);
				Planet->AddShipsDirectly(IncomingAttackLines[i].ShipAmount);

				//Construct entry for claim
				CombatResolutionResult = {RoundResolutionResultType_Combat, FString::Printf(TEXT("Player %s claimed planet %s."), *IncomingAttackLines[i].Player.Username, *Planet->GetPlanetName())};
			}
			else //Planet is owned by other player, resolve combat until one side runs out of ships
			{
				//Get fighting players
				FPlayerData Attacker = IncomingAttackLines[i].Player;
				FPlayerData Defender = Planet->GetOwningPlayer();
				
				//Combat
				int AttackingShips = IncomingAttackLines[i].ShipAmount;
				int DefendingShips = Planet->GetAvailableShipAmount();
				TArray<bool> AttackerWonList;
				while(IncomingAttackLines[i].ShipAmount > 0 && Planet->GetAvailableShipAmount() > 0)
				{
					//Roll until the players don't tie
					int AttackingPlayerRoll = 0;
					int DefendingPlayerRoll = 0;

					const int AttackerIndex = GetPlayerDataIndex(Attacker);
					const int DefenderIndex = GetPlayerDataIndex(Defender);

					const FPlayerData& AttackerDataRef = AllPlayers[AttackerIndex];
					const FPlayerData& DefenderDataRef = AllPlayers[DefenderIndex];
					
					while(AttackingPlayerRoll == DefendingPlayerRoll)
					{
						AttackingPlayerRoll = FMath::RandRange(1, 20) + AttackerDataRef.GetTechLevel();
						DefendingPlayerRoll = FMath::RandRange(1, 20) + DefenderDataRef.GetTechLevel();
					}

					//Remove one ship from defeated player
					bool AttackerWon = AttackingPlayerRoll > DefendingPlayerRoll;
					if(AttackerWon)
						Planet->RemoveShipsDirectly(1);
					else
						IncomingAttackLines[i].ShipAmount--;
					AttackerWonList.Add(AttackerWon);
				}

				//Grant planet to attacker if they won
				if(IncomingAttackLines[i].ShipAmount > 0)
				{
					Planet->SetOwningPlayer(IncomingAttackLines[i].Player);
					Planet->SetNewlyOvertaken(true);
					Planet->AddShipsDirectly(IncomingAttackLines[i].ShipAmount);
				}

				//Construct entry for combat
				FString CombatResult = IncomingAttackLines[i].ShipAmount > 0 ? "won" : "lost";
				FString ResultString = FString::Printf(TEXT("Player %s attacked player %s on planet %s and %s."), *Attacker.Username, *Defender.Username, *Planet->GetPlanetName(), *CombatResult);
				FCombatResult CombatResultInfo = {Attacker, Defender, Planet->GetPlanetIndex(), AttackingShips, DefendingShips, AttackerWonList};
				CombatResolutionResult = {RoundResolutionResultType_Combat, ResultString, CombatResultInfo};
			}
			
			//Add entry for combat
			for (int j = 0; j < NewResults.Num(); j++)
				NewResults[j].Results.Add(CombatResolutionResult);

			//Remove attack line
			Planet->RemoveIncomingAttackLine(IncomingAttackLines[i]);
		}
	}

	//Check if anyone won
	FPlayerData SomePlayer = Planets[0]->GetOwningPlayer();
	if(UHelperFunctions::All(Planets, [SomePlayer](const APlanet* Planet){ return Planet->IsOwnedByPlayer(SomePlayer); }))
	{
		FGameEndResult GameEndResult = {SomePlayer};
		FString ResultString = FString::Printf(TEXT("Player %s won the game by claiming all %d planets."), *SomePlayer.Username, Planets.Num());
		for (int i = 0; i < NewResults.Num(); i++)
			NewResults[i].Results.Add({RoundResolutionResultType_Combat, ResultString, GameEndResult});
	}
	
	//Send result to clients
	UE_LOG(LogTemp, Log, TEXT("SETTING NEW RESOLUTION RESULTS"))
	PlayersResolutionResults = NewResults;
}

void AGameManager::RegisterPlanet(APlanet* Planet)
{
	if(HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("REGISTERING PLANET SHOULD ONLY OCCUR ON CLIENT"))
		return;
	}
	
	Planets.AddUnique(Planet);
	OnPlanetListUpdated.Broadcast();
}

void AGameManager::AddGold(FPlayerData Player, const int Gold)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD GOLD OUTSIDE OF SERVER"))
		return;
	}

	//Find player
	const int Index = GetPlayerDataIndex(Player);
	if(Index < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("COULDN'T FIND PLAYER IN ADD GOLD"))
		return;
	}
	
	//Add gold
	FPlayerData& PlayerData = AllPlayers[Index];
	PlayerData.GoldAmount += Gold;
}

void AGameManager::RemoveGold(FPlayerData Player, const int Gold)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD GOLD OUTSIDE OF SERVER"))
		return;
	}

	//Find player
	const int Index = GetPlayerDataIndex(Player);
	if(Index < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("COULDN'T FIND PLAYER IN REMOVE GOLD"))
		return;
	}

	//Remove gold, don't go under 0
	FPlayerData& PlayerData = AllPlayers[Index];
	PlayerData.GoldAmount = FMath::Max(PlayerData.GoldAmount - Gold, 0);
}

void AGameManager::AddTechXP(FPlayerData Player, const float Xp)
{
	//Ensure this is performed on the server
	if(!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("TRYING TO ADD TECH XP OUTSIDE OF SERVER"))
		return;
	}

	//Find player
	const int Index = GetPlayerDataIndex(Player);
	if(Index < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("COULDN'T FIND PLAYER IN ADD TECH XP"))
		return;
	}

	//Add xp
	FPlayerData& PlayerData = AllPlayers[Index];
	PlayerData.TechLevel += Xp;
}

#pragma endregion

#pragma region Replication


void AGameManager::OnRep_GameStarted() const
{
	OnGameStateUpdated.Broadcast(GameStarted);
}

void AGameManager::OnRep_PlayersResolutionResults() const
{
	OnPlayersResolutionResultsUpdated.Broadcast();
}

void AGameManager::OnRep_AllPlayers(TArray<FPlayerData> PrevAllPlayers) const
{
	//Find changed values for delegates
	for(int i = 0; i < AllPlayers.Num(); i++)
	{
		//Ran out of prev entries, stop iterating
		if(PrevAllPlayers.Num() <= i)
			break;

		//Call delegate for OnGoldUpdated
		if(AllPlayers[i].GoldAmount != PrevAllPlayers[i].GoldAmount)
			OnGoldUpdated.Broadcast(AllPlayers[i], AllPlayers[i].GoldAmount);
	}
	
	OnPlayersUpdated.Broadcast();
}

#pragma endregion

int AGameManager::GetPlayerDataIndex(const FPlayerData& InPlayerData) const
{
	const int Index = AllPlayers.IndexOfByPredicate([InPlayerData](const FPlayerData& PlayerData){ return PlayerData == InPlayerData; });
	return Index;
}

TArray<APlanet*> AGameManager::GetPlanetsOwnedByPlayer(const FPlayerData& Player)
{
	TArray<APlanet*> OwnedPlanets;
	for (APlanet* Planet : Planets)
	{
		if(Planet->IsOwnedByPlayer(Player))
			OwnedPlanets.Add(Planet);
	}

	return OwnedPlanets;
}

APlanet* AGameManager::GetPlanetByName(const FString& InName)
{
	for (APlanet* Planet : Planets)
	{
		if(Planet->GetPlanetName() == InName)
			return Planet;
	}

	return nullptr;
}
