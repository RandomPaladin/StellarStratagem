#include "OnlineGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UOnlineGameInstance::ConnectToServer()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("192.168.0.27:7777")));
}