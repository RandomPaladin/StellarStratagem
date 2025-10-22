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
