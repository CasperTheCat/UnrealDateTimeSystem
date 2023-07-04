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
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float HourlyRainfall;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGameplayTagContainer MiscData;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemClimateOverrideRow &Row)
{
    auto DHash = GetTypeHash(Row.Day);
    auto MHash = GetTypeHash(Row.Month);
    auto YHash = GetTypeHash(Row.Year);

    auto Hash = HashCombine(YHash, MHash);
    return HashCombine(Hash, DHash);
}

FORCEINLINE uint32 GetDateHash(const FDateTimeSystemClimateOverrideRow &Row)
{
    return GetTypeHash(Row);
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
    float MonthlyHighTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float MonthlyLowTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float HourlyAverageRainfall;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemClimateMonthlyRow &Row)
{
    auto DHash = GetTypeHash(Row.MonthlyHighTemp);
    auto MHash = GetTypeHash(Row.MonthlyLowTemp);
    auto YHash = GetTypeHash(Row.DewPoint);

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

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGameplayTagContainer CallbackAttributes;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemDateOverrideRow &Row)
{
    auto DHash = GetTypeHash(Row.Day);
    auto MHash = GetTypeHash(Row.Month);
    auto YHash = GetTypeHash(Row.Year);

    auto Hash = HashCombine(YHash, MHash);
    return HashCombine(Hash, DHash);
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
    FText MonthName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int NumberOfDays;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool AffectedByLeap;
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemYearbookRow &Row)
{
    auto MIHash = GetTypeHash(Row.MonthName.ToString());

    return MIHash;
}
