// [TEMPLATE_COPYRIGHT]

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"

#include "DateTimeSystemDataRows.generated.h"


USTRUCT(BlueprintType)
struct FDateTimeSystemClimateOverrideRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FDateTimeSystemClimateOverrideRow()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Day;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Month;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Year;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HighTemp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float LowTemp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RelativeHumidity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer MiscData;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemClimateOverrideRow& Row)
{
	auto DHash = GetTypeHash(Row.Day);
	auto MHash = GetTypeHash(Row.Month);
	auto YHash = GetTypeHash(Row.Year);

	auto Hash = HashCombine(YHash, MHash);
	return HashCombine(Hash, DHash);
}




USTRUCT(BlueprintType)
struct FDateTimeSystemClimateMonthlyRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FDateTimeSystemClimateMonthlyRow()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Day;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Month;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Year;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MonthlyHighTemp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MonthlyLowTemp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RelativeHumidity;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemClimateMonthlyRow& Row)
{
	auto DHash = GetTypeHash(Row.Day);
	auto MHash = GetTypeHash(Row.Month);
	auto YHash = GetTypeHash(Row.Year);

	auto Hash = HashCombine(YHash, MHash);
	return HashCombine(Hash, DHash);
}






USTRUCT(BlueprintType)
struct FDateTimeSystemDateOverrideRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FDateTimeSystemDateOverrideRow()
	{
	}

	// When Game's DayIndex is equal. This overrides
	// Depending on GameState settings, it may also *set* the date to this
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int DayIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Day;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Month;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Year;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//float HighTemp;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//float LowTemp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer CallbackAttributes;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemDateOverrideRow& Row)
{
	auto DHash = GetTypeHash(Row.Day);
	auto MHash = GetTypeHash(Row.Month);
	auto YHash = GetTypeHash(Row.Year);

	auto Hash = HashCombine(YHash, MHash);
	return HashCombine(Hash, DHash);

	return Row.DayIndex;

	auto DIHash = GetTypeHash(Row.DayIndex);

	auto DateHash = HashCombine(HashCombine(DIHash, DHash), HashCombine(MHash, YHash));

	//auto HHash = GetTypeHash(Row.HighTemp);
	//auto LHash = GetTypeHash(Row.LowTemp);

	//auto TempHash = HashCombine(HHash, LHash);

	//auto CAHash = GetTypeHash(Row.CallbackAttributes);

	return DateHash;//HashCombine(DateHash, TempHash);
}





USTRUCT(BlueprintType)
struct FDateTimeSystemYearbookRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FDateTimeSystemYearbookRow()
	{
	}

	// Starts at zero
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName MonthName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int NumberOfDays;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool AffectedByLeap;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//float MonthlyHighTemp;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//float MonthlyLowTemp;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemYearbookRow& Row)
{
	auto MIHash = GetTypeHash(Row.MonthName);

	return MIHash;
}

