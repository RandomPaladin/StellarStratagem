#include "CombatAnimator.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

ACombatAnimator::ACombatAnimator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACombatAnimator::BeginPlay()
{
	Super::BeginPlay();
	
	//Get local player
	LocalPlayer = Cast<AStellarPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	//Get game manager
	GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	if(!GameManager)
	{
		UE_LOG(LogTemp, Error, TEXT("FOUND NO GAME MANAGER IN COMBAT ANIMATOR"))
		return;
	}
	
	//Ignore this class on the server
	if(GameManager->HasAuthority())
		return;

	//Bind to game manager
	GameManager->OnPlayersResolutionResultsUpdated.AddDynamic(this, &ACombatAnimator::DoCombatAnimations);
}

void ACombatAnimator::DoCombatAnimations()
{
	if(GameManager->GetIndexOfPlayersResolutionResults(LocalPlayer->GetPlayerData()) < 0)
		return;
	
	//Get results where combat info is given
	UHelperFunctions::Where(GameManager->GetPlayerResolutionResults(LocalPlayer->GetPlayerData()).Results, OUT Results, [](const FRoundResolutionResult& Result){ return Result.CombatResult.AttackingShips > 0; });

	//Ignore if no results
	if(Results.Num() == 0)
		return;
	
	ToggleUIUsability(false);
	ResultIndex = -1;
	IncrementAnimation();
}

void ACombatAnimator::IncrementAnimation()
{
	//Increment result index
	ResultIndex++;

	//Check if finished
	if(ResultIndex >= Results.Num())
	{
		ToggleUIUsability(true);
		return;
	}
	
	//Do animation
	DoCombatAnimation(Results[ResultIndex].CombatResult);
}