#include "PlanetGradeData.h"

int UPlanetGradeData::GenerateRandomBuildingSlotAmount(const EPlanetGrade Grade)
{
	//Get grade data
	FGradeData& GradeData = Grades[Grade];

	//Get distribution from curve
	TArray<float> Distribution;
	float Sum = 0.f;
	for (int i = GradeData.BuildingSlotsRange.X; i <= GradeData.BuildingSlotsRange.Y; i++)
	{
		float Value = GradeData.BuildingSlotsDistribution.GetRichCurve()->Eval(i);
		Sum += Value;
		Distribution.Add(Value);
	}

	//Normalize distribution
	for(int i = 0; i < Distribution.Num(); i++)
		Distribution[i] = Distribution[i] / Sum;

	//Determine random slot amount from distribution
	const float Roll = FMath::RandRange(0.f, 1.f);
	int Target = -1;
	float CurrentWeight = 0.f;
	for(int i = 0; i < Distribution.Num(); i++)
	{
		CurrentWeight += Distribution[i];

		//Continue until roll is found or end of distribution array is reached
		if(i < Distribution.Num() - 1 && Roll > CurrentWeight)
			continue;

		//Roll found or end reached, record target
		Target = i + GradeData.BuildingSlotsRange.X;
		break;
	}

	return Target;
}

void UPlanetGradeData::TestBuildingSlotDistribution()
{
	for (TTuple<TEnumAsByte<EPlanetGrade>, FGradeData> Kvp : Grades)
	{
		UE_LOG(LogTemp, Warning, TEXT("GRADE %d ======================="), (int)Kvp.Key);
		
		TMap<int, int> SlotAmounts;
		for(int i = 0; i < 1000; i++)
		{
			int SlotAmount = GenerateRandomBuildingSlotAmount(Kvp.Key);
			if(SlotAmounts.Contains(SlotAmount))
				SlotAmounts[SlotAmount]++;
			else
				SlotAmounts.Add(SlotAmount, 1);
		}
		
		SlotAmounts.KeySort([](auto Item1, auto Item2){ return Item1 < Item2; });

		for (TTuple<int, int> Amount : SlotAmounts)
			UE_LOG(LogTemp, Warning, TEXT("SLOT AMOUNT %d:\t%.1f %% OCCURENCE"), Amount.Key, (float)Amount.Value / 10.f);
	}
}
