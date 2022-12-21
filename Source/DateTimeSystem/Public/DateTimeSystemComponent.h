// [TEMPLATE_COPYRIGHT]

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "DateTimeSystemDataRows.h"
#include "GameplayTagContainer.h"

#include "DateTimeSystemComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("DateTimeSystem"), STATGROUP_ACIDateTimeSys, STATCAT_Advanced);

// Forward Decl
class UClimateComponent;

enum class EDateTimeSystemInvalidationTypes : uint8
{
    Frame,
    Day,
    Month,
    Year,

    TOTAL_INVALIDATION_TYPES
};

struct FDateTimeSystemPackedCacheFloat
{
    bool Valid;
    float Value;
};

struct FDateTimeSystemPackedCacheVector
{
    bool Valid;
    FVector Value;
};

struct FDateTimeSystemPackedCacheInt
{
    int32 Valid : 1;
    int32 Value : 31;
};

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
        this->DayIndex += Other.DayIndex;
        this->Month += Other.Month;
        this->Year += Other.Year;

        return *this;
    }

    friend FDateTimeSystemStruct operator+(FDateTimeSystemStruct Us, const FDateTimeSystemStruct &Other)
    {
        Us += Other;
        return Us;
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

USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemTimezoneStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float HoursDeltaFromMeridian;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDateChangeDelegate, UPARAM(ref) FDateTimeSystemStruct &, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOverridesDelegate, UPARAM(ref) FDateTimeSystemStruct &, NewDate,
                                             FGameplayTagContainer, Attribute);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UDateTimeSystemComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly)
    float LengthOfDay;

    double InvLengthOfDay;

    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    UPROPERTY(SaveGame, EditAnywhere)
    float TimeScale;

    UPROPERTY(EditDefaultsOnly)
    float DaysInOrbitalYear;

    UPROPERTY(EditDefaultsOnly)
    float ReferenceLatitude;

    float PercentLatitude;

    UPROPERTY(EditDefaultsOnly)
    float ReferenceLongitude;

    float PercentLongitude;

    UPROPERTY(EditDefaultsOnly)
    float SolsticeOffsetInDays;

    UPROPERTY(EditDefaultsOnly)
    float PlanetRadius;

    double InvPlanetRadius;

    UPROPERTY(EditDefaultsOnly)
    int DaysInWeek;

    UPROPERTY(EditDefaultsOnly)
    UDataTable *YearBookTable;

    UPROPERTY(EditDefaultsOnly)
    UDataTable *DateOverridesTable;

    // UTC Time. To avoid conversions
    UPROPERTY(SaveGame, EditDefaultsOnly)
    FDateTimeSystemStruct InternalDate;

    UPROPERTY(EditDefaultsOnly)
    bool UseDayIndexForOverride;

    // Used
    // UPROPERTY(SaveGame)
    // uint32 Day;

    // UPROPERTY(SaveGame)
    // uint32 Month;

    // UPROPERTY(SaveGame)
    // uint32 Year;

    // UPROPERTY(SaveGame)
    // uint32 DayIndex;

    // Create YearBook and DateOverrides + Array to force lifetimes
    TArray<FDateTimeSystemDateOverrideRow *> DOTemps;
    TMap<uint32, FDateTimeSystemDateOverrideRow *> DateOverrides;
    TArray<FDateTimeSystemYearbookRow *> YearBook;

    // Set when Yearbook is populated
    int LengthOfCalendarYearInDays;

    FDateTimeSystemPackedCacheFloat CachedSolarFractionalYear;
    FDateTimeSystemPackedCacheFloat CachedSolarDeclinationAngle;
    FDateTimeSystemPackedCacheFloat CachedSolarTimeCorrection;
    FDateTimeSystemPackedCacheFloat CachedSolarDaysOfYear;
    FDateTimeSystemPackedCacheInt CachedDoesLeap;
    TMap<uint32, FVector> CachedSunVectors;
    TMap<uint32, FVector> CachedMoonVectors;

public:
    // Do we *really* want to save this and not just bake it with EditDefaults?
    UPROPERTY(SaveGame, EditAnywhere)
    bool OverridedDatesSetDate;

    UPROPERTY(BlueprintAssignable)
    FDateChangeDelegate DateChangeCallback;

    UPROPERTY(BlueprintAssignable)
    FOverridesDelegate DateOverrideCallback;

private:
    void DateTimeSetup();
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

private:
    uint32 GetHashForDate(FDateTimeSystemStruct *DateStruct);
    uint32 GetHashForDate(FDateTimeSystemDateOverrideRow *DateStruct);
    FDateTimeSystemDateOverrideRow **GetDateOverride(FDateTimeSystemStruct *DateStruct);

    bool HandleDayRollover(FDateTimeSystemStruct &DateStruct);
    bool HandleMonthRollover(FDateTimeSystemStruct &DateStruct);
    bool HandleYearRollover(FDateTimeSystemStruct &DateStruct);

    int GetLengthOfCalendarYear(int Year);

    double GetJulianDay(FDateTimeSystemStruct &DateStruct);
    // float GetDay(FDateTimeSystemStruct& DateStruct);

    float GetDeclinationAngle(FDateTimeSystemStruct &DateStruct);

    int GetDaysInCurrentMonth();
    int GetDaysInMonth(int MonthIndex);
    int GetMonthsInYear(int YearIndex);

    bool SanitiseDateTime(FDateTimeSystemStruct &DateStruct);
    bool SanitiseSolarDateTime(FDateTimeSystemStruct &DateStruct);

    float GetSolarFractionalDay();
    float GetSolarFractionalYear();
    float SolarTimeCorrection(float YearInRadians);
    float SolarDeclinationAngle(float YearInRadians);

    bool InternalDoesLeap(int Year);

    virtual FRotator GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);
    virtual FRotator GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);

public:
    UDateTimeSystemComponent();
    UDateTimeSystemComponent(UDateTimeSystemComponent &Other);
    UDateTimeSystemComponent(const FObjectInitializer &ObjectInitializer);
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    virtual void BeginPlay();

    UFUNCTION(BlueprintCallable)
    void GetTodaysDate(FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    void GetTodaysDateTZ(FDateTimeSystemStruct &DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDate(FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDateTZ(FDateTimeSystemStruct &DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    UFUNCTION(BlueprintCallable)
    void GetYesterdaysDate(FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    void GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                             UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLatitudeFromLocation(float BaseLatitudePercent, FVector Location);
    virtual float GetLatitudeFromLocation_Implementation(float BaseLatitudePercent, FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLongitudeFromLocation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);
    virtual float GetLongitudeFromLocation_Implementation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                          FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    bool DoesYearLeap(int Year);
    virtual bool DoesYearLeap_Implementation(int Year);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotationForLocation(FVector Location);
    virtual FRotator GetSunRotationForLocation_Implementation(FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotation();
    virtual FRotator GetSunRotation_Implementation();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FVector GetSunVector(float Latitude, float Longitude);
    virtual FVector GetSunVector_Implementation(float Latitude, float Longitude);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetMoonRotationForLocation(FVector Location);
    virtual FRotator GetMoonRotationForLocation_Implementation(FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetMoonRotation();
    virtual FRotator GetMoonRotation_Implementation();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FVector GetMoonVector(float Latitude, float Longitude);
    virtual FVector GetMoonVector_Implementation(float Latitude, float Longitude);

    UFUNCTION(BlueprintCallable)
    float GetFractionalDay(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalOrbitalYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalCalendarYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void InternalBegin();

    UFUNCTION(BlueprintCallable)
    void InternalInitialise();

    UFUNCTION(BlueprintCallable)
    FVector AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection);

    UFUNCTION(BlueprintCallable)
    FName GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    // Set the thing, directly
    UFUNCTION(BlueprintCallable)
    void SetUTCDateTime(FDateTimeSystemStruct &DateStruct);

    // Return a copy of the internal struct
    // We make a copy to allow us to destroy the object without
    // risking a lifetime
    UFUNCTION(BlueprintCallable)
    FDateTimeSystemStruct GetUTCDateTime();

    UFUNCTION(BlueprintCallable)
    void AddDateStruct(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    friend class UClimateComponent;
};
