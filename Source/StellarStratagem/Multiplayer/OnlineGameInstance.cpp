#include "OnlineGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UOnlineGameInstance::ConnectToServer()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("192.168.0.14:7777")));
}

bool UOnlineGameInstance::IsConnected()
{
	ENetMode NetMode = GetWorld()->GetNetMode();
	return NetMode == NM_Client;
}
