// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Day;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Month;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Year;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float HighTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float LowTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float HourlyRainfall;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FGameplayTagContainer MiscData;
};

// FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemClimateOverrideRow &Row)
//{
//     auto DHash = GetTypeHash(Row.Day);
//     auto MHash = GetTypeHash(Row.Month);
//     auto YHash = GetTypeHash(Row.Year);
//
//     auto Hash = HashCombine(YHash, MHash);
//     return HashCombine(Hash, DHash);
// }
//
// FORCEINLINE uint32 GetDateHash(const FDateTimeSystemClimateOverrideRow &Row)
//{
//     return GetTypeHash(Row);
// }

UCLASS(BlueprintType, Blueprintable)
class UDateTimeSystemClimateOverrideItem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Day;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Month;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Year;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float HighTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float LowTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float HourlyRainfall;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FGameplayTagContainer MiscData;
};

FORCEINLINE uint32 GetTypeHash(const UDateTimeSystemClimateOverrideItem *Row)
{
    auto DHash = GetTypeHash(Row->Day);
    auto MHash = GetTypeHash(Row->Month);
    auto YHash = GetTypeHash(Row->Year);

    auto Hash = HashCombine(YHash, MHash);
    return HashCombine(Hash, DHash);
}

FORCEINLINE uint32 GetDateHash(const UDateTimeSystemClimateOverrideItem *Row)
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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float MonthlyHighTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float MonthlyLowTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
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

UCLASS(BlueprintType, Blueprintable)
class UDateTimeSystemClimateMonthlyItem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float MonthlyHighTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float MonthlyLowTemp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float DewPoint;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float RainfallProbability;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    float HourlyAverageRainfall;
};

FORCEINLINE uint32 GetTypeHash(const UDateTimeSystemClimateMonthlyItem *Row)
{
    auto DHash = GetTypeHash(Row->MonthlyHighTemp);
    auto MHash = GetTypeHash(Row->MonthlyLowTemp);
    auto YHash = GetTypeHash(Row->DewPoint);

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
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int DayIndex;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Day;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Month;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Year;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FGameplayTagContainer CallbackAttributes;
};

// FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemDateOverrideRow &Row)
//{
//     auto DHash = GetTypeHash(Row.Day);
//     auto MHash = GetTypeHash(Row.Month);
//     auto YHash = GetTypeHash(Row.Year);
//
//     auto Hash = HashCombine(YHash, MHash);
//     return HashCombine(Hash, DHash);
// }

USTRUCT(BlueprintType)
struct FDateTimeSystemYearbookRow : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

public:
    FDateTimeSystemYearbookRow()
    {
    }

    // Starts at zero
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FText MonthName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int NumberOfDays;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    bool AffectedByLeap;
};

// FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemYearbookRow &Row)
//{
//     auto MIHash = GetTypeHash(Row.MonthName.ToString());
//
//     return MIHash;
// }

UCLASS(BlueprintType, Blueprintable)
class UDateTimeSystemYearbookItem : public UObject
{
    GENERATED_BODY()

public:
    // Starts at zero
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FText MonthName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int NumberOfDays;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    bool AffectedByLeap;
};

FORCEINLINE uint32 GetTypeHash(const UDateTimeSystemYearbookItem *Row)
{
    auto MIHash = GetTypeHash(Row->MonthName.ToString());

    return MIHash;
}

UCLASS(BlueprintType, Blueprintable)
class UDateTimeSystemDateOverrideItem : public UObject
{
    GENERATED_BODY()

public:
    // When Game's DayIndex is equal. This overrides
    // Depending on GameState settings, it may also *set* the date to this
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int DayIndex;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Day;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Month;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    int Year;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Date and Time")
    FGameplayTagContainer CallbackAttributes;
};

FORCEINLINE uint32 GetTypeHash(const UDateTimeSystemDateOverrideItem *Row)
{
    auto DHash = GetTypeHash(Row->Day);
    auto MHash = GetTypeHash(Row->Month);
    auto YHash = GetTypeHash(Row->Year);

    auto Hash = HashCombine(YHash, MHash);
    return HashCombine(Hash, DHash);
}

struct DateTimeRowHelpers
{
#define ASSIGN_MEMBER(ObjectName, MemberName) ObjectName->MemberName = Row->MemberName

    static FORCEINLINE TObjectPtr<UDateTimeSystemYearbookItem> CreateYearbookRowFromTableRow(
        const FDateTimeSystemYearbookRow *Row)
    {
        auto Object = NewObject<UDateTimeSystemYearbookItem>();
        Object->MonthName = Row->MonthName;
        Object->NumberOfDays = Row->NumberOfDays;
        Object->AffectedByLeap = Row->AffectedByLeap;

        return Object;
    }

    static FORCEINLINE TObjectPtr<UDateTimeSystemDateOverrideItem> CreateOverrideItemFromTableRow(
        const FDateTimeSystemDateOverrideRow *Row)
    {
        auto Object = NewObject<UDateTimeSystemDateOverrideItem>();
        Object->DayIndex = Row->DayIndex;
        Object->Day = Row->Day;
        Object->Month = Row->Month;
        Object->Year = Row->Year;
        Object->CallbackAttributes = Row->CallbackAttributes;

        return Object;
    }

    static FORCEINLINE TObjectPtr<UDateTimeSystemClimateMonthlyItem> CreateClimateMonthlyFromTableRow(
        const FDateTimeSystemClimateMonthlyRow *Row)
    {
        auto Object = NewObject<UDateTimeSystemClimateMonthlyItem>();
        ASSIGN_MEMBER(Object, MonthlyHighTemp);
        ASSIGN_MEMBER(Object, MonthlyLowTemp);
        ASSIGN_MEMBER(Object, DewPoint);
        ASSIGN_MEMBER(Object, RainfallProbability);
        ASSIGN_MEMBER(Object, HourlyAverageRainfall);

        return Object;
    }

    static FORCEINLINE TObjectPtr<UDateTimeSystemClimateOverrideItem> CreateClimateMonthlyOverrideFromTableRow(
        const FDateTimeSystemClimateOverrideRow *Row)
    {
        auto Object = NewObject<UDateTimeSystemClimateOverrideItem>();
        ASSIGN_MEMBER(Object, Day);
        ASSIGN_MEMBER(Object, Month);
        ASSIGN_MEMBER(Object, Year);
        ASSIGN_MEMBER(Object, HighTemp);
        ASSIGN_MEMBER(Object, LowTemp);
        ASSIGN_MEMBER(Object, DewPoint);
        ASSIGN_MEMBER(Object, RainfallProbability);
        ASSIGN_MEMBER(Object, HourlyRainfall);
        ASSIGN_MEMBER(Object, MiscData);

        return Object;
    }
};
