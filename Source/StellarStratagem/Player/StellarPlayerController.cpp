#include "StellarPlayerController.h"

void AStellarPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(IsLocalController())
	{
		auto Data = FActionData{8};
		UE_LOG(LogTemp, Warning, TEXT("SENDING NUM %d"), Data.NumberTest);
		SendAction(Data);
	}
}

void AStellarPlayerController::Receive_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("RECEIVED FROM SERVER"));
}

void AStellarPlayerController::SendAction_Implementation(const FActionData& ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("RECEIVED NUM %d"), ActionData.NumberTest);
	Receive();
}
