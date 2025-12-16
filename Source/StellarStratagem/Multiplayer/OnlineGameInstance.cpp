#include "OnlineGameInstance.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "StellarStratagem/Player/StellarPlayerController.h"

void UOnlineGameInstance::CreateGame(AStellarPlayerController* Player, const FString& GameCode)
{
	InstigatingPlayer = Player;
	
	const TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject());
	JsonBody->SetStringField("game_code", GameCode);
	FString RequestBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	SendRequestToServer(RequestBody, "create-game", &UOnlineGameInstance::OnGameCreated);
}

void UOnlineGameInstance::JoinGame(AStellarPlayerController* Player, const FString& GameCode)
{
	InstigatingPlayer = Player;
	
	const TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject());
	JsonBody->SetStringField("game_code", GameCode);
	FString RequestBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	SendRequestToServer(RequestBody, "join-game", &UOnlineGameInstance::OnGameCreated);
}

void UOnlineGameInstance::ShutdownGame(const FString& GameCode)
{
	UE_LOG(LogTemp, Warning, TEXT("TRYING TO SHUT DOWN GAME: %s"), *GameCode)
	
	const TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject());
	JsonBody->SetStringField("game_code", GameCode);
	FString RequestBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	SendRequestToServer(RequestBody, "shutdown-game", &UOnlineGameInstance::OnGameShutDown);
}

void UOnlineGameInstance::SendRequestToServer(const FString& RequestBody, const FString& FunctionName, void(UOnlineGameInstance::* Callback)(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success))
{
	//Create request
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s:%s/%s"), *ServerIpAddress, *ServerEndpointPort, *FunctionName));
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);

	//Send request
	Request->OnProcessRequestComplete().BindUObject(this, Callback);
	Request->ProcessRequest();
}

void UOnlineGameInstance::OnGameCreated(FHttpRequestPtr Request, FHttpResponsePtr Response, const bool Success)
{
	if (!Success || !Response.IsValid())
		return;

	//Get content
	const FString Body = Response->GetContentAsString();
	TSharedPtr<FJsonObject> Json;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
		return;

	//Handle errors
	if(!Json->HasField("ip") || !Json->HasField("port"))
	{
		//Show error to player if one was given
		if(Json->HasField("error") && InstigatingPlayer)
			InstigatingPlayer->ShowMessage(Json->GetStringField("error"));
			
		return;
	}

	//Get target address
	const FString Ip = Json->GetStringField("ip");
	const int Port = Json->GetIntegerField("port");

	//Connect to server
	const FString URL = FString::Printf(TEXT("%s:%d"), *Ip, Port);
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientTravel(URL, TRAVEL_Absolute);
}

void UOnlineGameInstance::OnGameShutDown(FHttpRequestPtr Request, FHttpResponsePtr Response, bool Success)
{
	if (!Success || !Response.IsValid())
		return;
		
	//Get content
	const FString Body = Response->GetContentAsString();
	TSharedPtr<FJsonObject> Json;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
		return;

	//Handle errors
	if(Json->HasField("error"))
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: %s"), *Json->GetStringField("error"))
		return;
	}

	//Close server
	UE_LOG(LogTemp, Warning, TEXT("CLOSING SERVER"))
	FPlatformMisc::RequestExit(false);
}

bool UOnlineGameInstance::IsConnected() const
{
	return GetWorld()->GetNetMode() == NM_Client;
}
