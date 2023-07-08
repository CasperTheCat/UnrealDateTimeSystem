// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DateTimeSystemDataRows.h"
#include "GameplayTagContainer.h"

#include "DateTimeTypes.generated.h"

#ifndef DATETIMESYSTEM_POINTERCHECK
#define DATETIMESYSTEM_POINTERCHECK (!(UE_BUILD_SHIPPING || UE_BUILD_TEST) || WITH_EDITOR)
#endif

struct DateTimeHelpers
{
    static FORCEINLINE float HelperMod(double X, double Y)
    {
        const double AbsY = FMath::Abs(Y);
        if (AbsY <= 1.e-8)
        {
            return 0.0;
        }

        const double Div = (X / Y);
        double Frac = FMath::Fractional(Div);

        if (Frac < 0)
        {
            Frac += 1;
        }

        const double Result = Y * Frac;
        return Result;
    }

    static FORCEINLINE int IntHelperMod(int X, int Y)
    {
        return ((X %= Y) < 0) ? X + Y : X;
    }
};

/**
 * @brief Date Time Cache Invalidation Types
 *
 */
UENUM(BlueprintType)
enum class EDateTimeSystemInvalidationTypes : uint8
{
    Frame,
    Day,
    Month,
    Year,

    TOTAL_INVALIDATION_TYPES UMETA(Hidden)
};

/**
 * @brief Cache Float
 *
 * Includes Validity
 * Packing does nothing on this type
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemPackedCacheFloat
{
    GENERATED_BODY()

    bool Valid;
    float Value;
};

/**
 * @brief Cache Vector
 *
 * Includes Validity
 * Packing does nothing on this type
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemPackedCacheVector
{
    GENERATED_BODY()

    bool Valid;
    FVector Value;
};

/**
 * @brief Cache Integer
 *
 * Includes Validity
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemPackedCacheInt
{
    GENERATED_BODY()

    int32 Valid : 1;
    int32 Value : 31;
};

/**
 * @brief Date Time Struct
 *
 * Stores time in UTC
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemStruct
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float Seconds;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    int Day;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    int Month;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    int Year;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    int DayOfWeek;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    int DayIndex;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    int SolarDays;

    UPROPERTY(SaveGame, BlueprintReadWrite)
    float StoredSolarSeconds;

public:
    float GetTimeInMinutes()
    {
        return Seconds / 60;
    }

    uint32 GetBinHash(float LengthOfDay, int NumberOfBinsPerDay = 24)
    {
        auto YH = GetTypeHash(Year);
        auto MH = GetTypeHash(Month);
        auto DH = GetTypeHash(Day);

        auto HourBin = FMath::TruncToInt32(Seconds / (LengthOfDay / NumberOfBinsPerDay));
        auto HH = GetTypeHash(HourBin);

        auto YMH = HashCombine(YH, MH);
        auto DHH = HashCombine(DH, HH);
        auto YMDHH = HashCombine(YMH, DHH);

        return YMDHH;
    }

    int32 GetHourBin(float LengthOfDay, int NumberOfBinsPerDay = 24)
    {
        return FMath::TruncToInt32(Seconds / (LengthOfDay / NumberOfBinsPerDay));
    }

    float GetFractionalBin(float LengthOfDay, int NumberOfBinsPerDay = 24)
    {
        return FMath::Frac(Seconds / (LengthOfDay / NumberOfBinsPerDay));
    }

    void SetFromRow(FDateTimeSystemDateOverrideRow *Row)
    {
        Seconds = 0;
        Day = Row->Day;
        Month = Row->Month;
        Year = Row->Year;
    }

    static FDateTimeSystemStruct CreateFromRow(FDateTimeSystemDateOverrideRow *Row)
    {
        auto RetVal = FDateTimeSystemStruct{};
        RetVal.Seconds = 0;
        RetVal.Day = Row->Day;
        RetVal.Month = Row->Month;
        RetVal.Year = Row->Year;
        RetVal.DayIndex = Row->DayIndex;

        return RetVal;
    }

    FDateTimeSystemStruct &operator+=(const FDateTimeSystemStruct &Other)
    {
        this->Seconds += Other.Seconds;
        this->Day += Other.Day;
        this->Month += Other.Month;
        this->Year += Other.Year;

        return *this;
    }

    FDateTimeSystemStruct &operator-=(const FDateTimeSystemStruct &Other)
    {
        this->Seconds -= Other.Seconds;
        this->Day -= Other.Day;
        this->Month -= Other.Month;
        this->Year -= Other.Year;

        return *this;
    }

    friend FDateTimeSystemStruct operator+(const FDateTimeSystemStruct &Us, const FDateTimeSystemStruct &Other)
    {
        FDateTimeSystemStruct New = Us;
        New += Other;
        return New;
    }

    friend FDateTimeSystemStruct operator-(const FDateTimeSystemStruct &Us, const FDateTimeSystemStruct &Other)
    {
        FDateTimeSystemStruct New = Us;
        New -= Other;
        return New;
    }

    friend bool operator>(const FDateTimeSystemStruct &lhs,
                          const FDateTimeSystemStruct &rhs) // friend claim has to be here
    {
        if (lhs.Year != rhs.Year)
        {
            return lhs.Year > rhs.Year;
        }

        if (lhs.Month != rhs.Month)
        {
            return lhs.Month > rhs.Month;
        }

        if (lhs.Day != rhs.Day)
        {
            return lhs.Day > rhs.Month;
        }

        return lhs.Seconds > rhs.Seconds;
    }

    uint32 GetHash()
    {
        auto SHash = GetTypeHash(Seconds);
        auto DHash = GetTypeHash(Day);
        auto MHash = GetTypeHash(Month);
        auto YHash = GetTypeHash(Year);

        auto DateHash = HashCombine(HashCombine(SHash, DHash), HashCombine(MHash, YHash));

        return DateHash;
    }

    FDateTime &GetDateTime()
    {
        auto _Hour = FMath::TruncToInt32(Seconds / 3600);
        auto _Minute = FMath::TruncToInt32(DateTimeHelpers::HelperMod(Seconds, 3600) / 60);
        auto _Seconds = FMath::TruncToInt32(DateTimeHelpers::HelperMod(Seconds, 60));
        auto Milli = FMath::TruncToInt32(1000 * FMath::Frac(Seconds));

        auto UEDateTime = FDateTime(Year, Month + 1, Day + 1, _Hour, _Minute, _Seconds, Milli);

        return UEDateTime;
    }
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemStruct &Row)
{
    return Row.DayIndex;
    auto DIHash = GetTypeHash(Row.DayIndex);
    auto DHash = GetTypeHash(Row.Day);
    auto MHash = GetTypeHash(Row.Month);
    auto YHash = GetTypeHash(Row.Year);

    auto DateHash = HashCombine(HashCombine(DIHash, DHash), HashCombine(MHash, YHash));

    return DateHash;
}

FORCEINLINE uint32 GetDateHash(const FDateTimeSystemStruct &Row)
{
    auto DHash = GetTypeHash(Row.Day);
    auto MHash = GetTypeHash(Row.Month);
    auto YHash = GetTypeHash(Row.Year);

    auto Hash = HashCombine(YHash, MHash);
    return HashCombine(Hash, DHash);
}

/**
 * @brief Timezone Struct
 *
 * Does not handle Daylight Saving
 * Dumb, raw offset to hours
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemTimezoneStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float HoursDeltaFromMeridian;
};

DATETIMESYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogDateTimeSystem, Log, All);
DATETIMESYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogClimateSystem, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleanDateChangeDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDateChangeDelegate, FDateTimeSystemStruct, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOverridesDelegate, FDateTimeSystemStruct, NewDate, FGameplayTagContainer,
                                             Attribute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvalidationDelegate, EDateTimeSystemInvalidationTypes, InvalidationType);

/**
 * @brief Cache Float
 *
 * Includes Validity
 * Packing does nothing on this type
 */
USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeCommonCoreInitializer
{
    GENERATED_BODY()

    UPROPERTY()
    float LengthOfDay;

    UPROPERTY()
    float DaysInOrbitalYear;

    UPROPERTY()
    UDataTable *YearbookTable;

    UPROPERTY()
    UDataTable *DateOverridesTable;

    UPROPERTY()
    bool UseDayIndexForOverride;

    UPROPERTY()
    float PlanetRadius;

    UPROPERTY()
    float ReferenceLatitude;

    UPROPERTY()
    float ReferenceLongitude;

    UPROPERTY()
    FDateTimeSystemStruct StartDate;

    UPROPERTY()
    int DaysInWeek;

    UPROPERTY()
    bool OverridedDatesSetDate;
};
