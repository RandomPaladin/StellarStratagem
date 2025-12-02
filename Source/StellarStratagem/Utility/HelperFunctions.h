#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HelperFunctions.generated.h"

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
