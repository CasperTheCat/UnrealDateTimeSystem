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

/**
 * @brief Date Time Cache Invalidation Types
 *
 */
enum class EDateTimeSystemInvalidationTypes : uint8
{
    Frame,
    Day,
    Month,
    Year,

    TOTAL_INVALIDATION_TYPES
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

    friend FDateTimeSystemStruct operator+(FDateTimeSystemStruct Us, const FDateTimeSystemStruct &Other)
    {
        Us += Other;
        return Us;
    }

    friend FDateTimeSystemStruct operator-(FDateTimeSystemStruct Us, const FDateTimeSystemStruct &Other)
    {
        Us -= Other;
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

    UPROPERTY(BlueprintReadWrite)
    float HoursDeltaFromMeridian;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDateChangeDelegate, FDateTimeSystemStruct, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOverridesDelegate, FDateTimeSystemStruct, NewDate, FGameplayTagContainer,
                                             Attribute);

/**
 * @brief DateTimeSystem
 *
 * Subclasses UActorComponent so it can be placed in more locations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UDateTimeSystemComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    /**
     * @brief Length of a Day in Seconds
     * This refers to both Solar and Calendar days
     */
    UPROPERTY(EditDefaultsOnly)
    float LengthOfDay;

    /**
     * @brief Runtime Reciprocal for day length in seconds
     *
     */
    UPROPERTY()
    double InvLengthOfDay;

    /**
     * @brief Sets how many times per second this object should tick
     * Only used if BeginPlay is called by the engine
     *
     * Set to Zero (0) to use default
     */
    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    /**
     * @brief Scale the input DeltaTime
     *
     */
    UPROPERTY(SaveGame, EditAnywhere)
    float TimeScale;

    /**
     * @brief Number of days, including any fractional component, in a year
     * Calendar Years are populated
     */
    UPROPERTY(EditDefaultsOnly)
    float DaysInOrbitalYear;

    /**
     * @brief Reference Latitude
     * All Latitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditDefaultsOnly)
    float ReferenceLatitude;

    /**
     * @brief Latitude as a percentage
     * For ease of use internally
     */
    UPROPERTY()
    float PercentLatitude;

    /**
     * @brief Reference Longitude
     * All Longitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditDefaultsOnly)
    float ReferenceLongitude;

    /**
     * @brief Longitude as a percentage
     * For ease of use internally
     */
    UPROPERTY()
    float PercentLongitude;

    /**
     * @brief Planet Radius in Kilometres
     * Used for computing sun and moon location at vector positions
     */
    UPROPERTY(EditDefaultsOnly)
    float PlanetRadius;

    /**
     * @brief Runtime Reciprocal for Planet Radius
     */
    UPROPERTY()
    double InvPlanetRadius;

    /**
     * @brief Number of Days in a week
     * Used for rolling over the day-of-week field
     *
     */
    UPROPERTY(EditDefaultsOnly)
    int DaysInWeek;

    /**
     * @brief Yearbook
     * Uses the FDateTimeSystemYearbookRow row schema
     */
    UPROPERTY(EditDefaultsOnly)
    UDataTable *YearBookTable;

    /**
     * @brief Overrides
     * Uses the FDateTimeSystemDateOverrideRow row schema
     */
    UPROPERTY(EditDefaultsOnly)
    UDataTable *DateOverridesTable;

    /**
     * @brief Whether set the overriden values when the date matches the current date
     * or the override dayindex matches the current dayindex
     *
     * Defaults to matching the date
     */
    UPROPERTY(EditDefaultsOnly)
    bool UseDayIndexForOverride;

    /**
     * @brief Array holding DateOverrides
     *
     */
    TArray<FDateTimeSystemDateOverrideRow *> DOTemps;

    /**
     * @brief Map from value to DateOverrides
     * Value is dictated by the UseDayIndexForOverride function
     *
     */
    TMap<uint32, FDateTimeSystemDateOverrideRow *> DateOverrides;

    /**
     * @brief Array holding Yearbook rows
     *
     */
    TArray<FDateTimeSystemYearbookRow *> YearBook;

    /**
     * @brief Length of a year in calendar days
     *
     */
    UPROPERTY()
    int LengthOfCalendarYearInDays;

    // Caches
    /**
     * @brief Cache for Solar Fractional Year
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedSolarFractionalYear;

    /**
     * @brief Cache for Solar Declination Angle
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedSolarDeclinationAngle;

    /**
     * @brief Cache for Solar Declination Angle
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedSolarTimeCorrection;

    /**
     * @brief Cache for Solar Days in Year
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedSolarDaysOfYear;

    /**
     * @brief Cache for whether this year leaps
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheInt CachedDoesLeap;

    /**
     * @brief Array of cached Sun Locations
     *
     * This makes querying sun position at the same location and time relatively cheap
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, FVector> CachedSunVectors;

    /**
     * @brief Array of cached Moon Locations
     *
     * This makes querying moon position at the same location and time relatively cheap
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, FVector> CachedMoonVectors;

    /**
     * @brief Internal Date and Time stored in UTC
     *
     */
    UPROPERTY(SaveGame, EditDefaultsOnly)
    FDateTimeSystemStruct InternalDate;

public:
    /**
     * @brief When an override occurs, do we want to update the date to the date in the override row?
     *
     * Note: This only makes sense when UseDayIndexForOverride is true,
     * which it is not by default.
     */
    UPROPERTY(SaveGame, EditAnywhere)
    bool OverridedDatesSetDate;

    /**
     * @brief Callback when the date changes
     */
    UPROPERTY(BlueprintAssignable)
    FDateChangeDelegate DateChangeCallback;

    /**
     * @brief Callback when an override occurs
     */
    UPROPERTY(BlueprintAssignable)
    FOverridesDelegate DateOverrideCallback;

    /**
     * @brief Callback when the time changes, which may be frequently
     *
     * Also used by Climate Components to trigger their updates
     */
    UPROPERTY(BlueprintAssignable)
    FDateChangeDelegate TimeUpdate;

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
    /**
     * @brief Construct a new UDateTimeSystemComponent object
     */
    UDateTimeSystemComponent();
    UDateTimeSystemComponent(UDateTimeSystemComponent &Other);
    UDateTimeSystemComponent(const FObjectInitializer &ObjectInitializer);

    /**
     * @brief Engine tick function. Called when this component is on a tickable actor.
     * If it isn't, call InternalTick manually.
     *
     * @param DeltaTime
     * @param TickType
     * @param ThisTickFunction
     */
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    /**
     * @brief Engine Begin Function. Called by the engine on a tickable actor.
     * If it isn't, call InternalBegin manually.
     */
    virtual void BeginPlay();

    ///// ///// ////////// ///// /////
    // Date Functions
    //

    /**
     * @brief Populate DateStruct with today's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    void GetTodaysDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Populate DateStruct with today's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    void GetTodaysDateTZ(FDateTimeSystemStruct &DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    /**
     * @brief Populate DateStruct with tomorrow's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Populate DateStruct with tomorrow's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDateTZ(FDateTimeSystemStruct &DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    /**
     * @brief Populate DateStruct with yesterday's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    void GetYesterdaysDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Populate DateStruct with yesterday's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    void GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                             UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo);

    /**
     * @brief Set DTS date
     * Use SkipInitialisation if we are loading a checkpoint
     *
     */
    UFUNCTION(BlueprintCallable)
    void SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation = false);

    /**
     * @brief Return a copy of the internal struct
     * Useful for saving the state
     *
     */
    UFUNCTION(BlueprintCallable)
    FDateTimeSystemStruct GetUTCDateTime();

    /**
     * Functions for Adding and Setting time in increments
     */

    UFUNCTION(BlueprintCallable)
    void AddDateStruct(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    void AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    bool AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety = true);

    // Misc
    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &From,
                                         UPARAM(ref) FDateTimeSystemStruct &To);

    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    // Double Precision Delta
    double DComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &From,
                                            UPARAM(ref) FDateTimeSystemStruct &To);

    TTuple<float, float, float> ComputeDeltaBetweenDatesInternal(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                 UPARAM(ref) FDateTimeSystemStruct &Date2,
                                                                 FDateTimeSystemStruct &Result);

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

    /**
     * @brief This function is used if the component cannot tick itself.
     * Such as, for example, when placed on a GameInstance.
     */
    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime);

    /**
     * @brief Called by BeginPlay
     * Can be called if the component isn't receiving a BeginPlay
     * Such as when on a GameInstance
     */
    UFUNCTION(BlueprintCallable)
    void InternalBegin();

    /**
     * @brief Reinitialise
     *
     * This probably doesn't need to be called unless you know why
     * Current functions that require reinitialisation already call this
     */
    UFUNCTION(BlueprintCallable)
    void InternalInitialise();

    UFUNCTION(BlueprintCallable)
    FVector AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection);

    /**
     * @brief Return the FName of the month
     *
     * Remember, FName's are not localised, and this isn't using FText, which is
     * Using the FName directly in the UI is not advised
     */
    UFUNCTION(BlueprintCallable)
    FText GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetTimeScale()
    {
        return TimeScale;
    }

    friend class UClimateComponent;
};
