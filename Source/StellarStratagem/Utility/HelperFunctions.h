#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HelperFunctions.generated.h"

class ULocalUserSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoParamDelegate);

UCLASS()
class STELLARSTRATAGEM_API UHelperFunctions : public UObject
{
	GENERATED_BODY()

public:
	template<class T, typename Predicate> 
	static void Where(TArray<T> InArray, OUT TArray<T>& OutArray, Predicate Pred);

	template<class T, typename Predicate> 
	static bool Any(TArray<T> InArray, Predicate Pred);

	template<class T, typename Predicate> 
	static bool All(TArray<T> InArray, Predicate Pred);

	template<class T, typename Predicate> 
	static bool First(const TArray<T> InArray, Predicate Pred, T& OutValue);
	
	static void SaveLocalUserData(const FString& Username);
	static ULocalUserSaveGame* GetLocalUserData();
};

template <class T, typename Predicate>
void UHelperFunctions::Where(TArray<T> InArray, OUT TArray<T>& OutArray, Predicate Pred)
{
	OutArray.Empty();
	for (T Entry : InArray)
	{
		if(Pred(Entry))
			OutArray.Add(Entry);
	}
}

template <class T, typename Predicate>
bool UHelperFunctions::Any(TArray<T> InArray, Predicate Pred)
{
	for (T Entry : InArray)
	{
		if(Pred(Entry))
			return true;
	}

	return false;
}

template <class T, typename Predicate>
bool UHelperFunctions::All(TArray<T> InArray, Predicate Pred)
{
	for (T Entry : InArray)
	{
		if(!Pred(Entry))
			return false;
	}

	return true;
}

template <class T, typename Predicate>
bool UHelperFunctions::First(const TArray<T> InArray, Predicate Pred, T& OutValue)
{
	int Index = InArray.IndexOfByPredicate(Pred);
	if (Index < 0)
		return false;

	OutValue = InArray[Index];
	return true;
}
