#include "OnlineGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UOnlineGameInstance::ConnectToServer() const
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("192.168.0.192:7777")));
}

bool UOnlineGameInstance::IsConnected() const
{
	ENetMode NetMode = GetWorld()->GetNetMode();
	return NetMode == NM_Client;
}
