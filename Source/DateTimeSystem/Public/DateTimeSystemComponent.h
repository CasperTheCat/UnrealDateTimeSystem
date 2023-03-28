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

    uint32 GetHash()
    {
        auto SHash = GetTypeHash(Seconds);
        auto DHash = GetTypeHash(Day);
        auto MHash = GetTypeHash(Month);
        auto YHash = GetTypeHash(Year);

        auto DateHash = HashCombine(HashCombine(SHash, DHash), HashCombine(MHash, YHash));

        return DateHash;
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleanDateChangeDelegate);
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
    TObjectPtr<UDataTable> YearBookTable;

    /**
     * @brief Overrides
     * Uses the FDateTimeSystemDateOverrideRow row schema
     */
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UDataTable> DateOverridesTable;

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
    UPROPERTY(Transient)
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
     */
    UPROPERTY(BlueprintAssignable)
    FDateChangeDelegate TimeUpdate;

    /**
     * @brief Callback when the time changes, which may be frequently
     *
     * Also used by Climate Components to trigger their updates
     */
    UPROPERTY(BlueprintAssignable)
    FCleanDateChangeDelegate CleanTimeUpdate;

private:
    /**
     * @brief Called by the constructors, and nothing else
     *
     */
    void DateTimeSetup();

    /**
     * @brief Invalidate the caches based on the Type of Invalidation
     *
     * @param Type
     */
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

private:
    /**
     * @brief Get the Hash For Date object
     *
     * @param DateStruct
     * @return uint32
     */
    uint32 GetHashForDate(FDateTimeSystemStruct *DateStruct);

    /**
     * @brief Get the Hash For Date object
     *
     * @param DateStruct
     * @return uint32
     */
    uint32 GetHashForDate(FDateTimeSystemDateOverrideRow *DateStruct);

    /**
     * @brief Get the Date Override object
     *
     * @param DateStruct
     * @return FDateTimeSystemDateOverrideRow**
     */
    FDateTimeSystemDateOverrideRow **GetDateOverride(FDateTimeSystemStruct *DateStruct);

    /**
     * @brief Get the Julian Day
     *
     * @param DateStruct
     * @return double
     */
    double GetJulianDay(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the number of days In current month
     *
     * @return int
     */
    int GetDaysInCurrentMonth();

    /**
     * @brief Get the number of days in the specified month by index
     *
     * @param MonthIndex
     * @return int
     */
    int GetDaysInMonth(int MonthIndex);

    /**
     * @brief Get the count of months In a year
     *
     * @param YearIndex
     * @return int
     */
    int GetMonthsInYear(int YearIndex);

    /**
     * @brief Get the Length Of a Calendar Year
     *
     * @param Year
     * @return int
     */
    int GetLengthOfCalendarYear(int Year);

    /**
     * @brief Get the Solar Fractional Day
     *
     * @return float
     */
    float GetSolarFractionalDay();

    /**
     * @brief Get the Solar Fractional Year
     *
     * @return float
     */
    float GetSolarFractionalYear();

    /**
     * @brief Get EQ Time
     *
     * @param YearInRadians
     * @return float
     */
    float SolarTimeCorrection(float YearInRadians);

    /**
     * @brief Get Solar Declination Angle
     *
     * @param YearInRadians
     * @return float
     */
    float SolarDeclinationAngle(float YearInRadians);

    /**
     * @brief Does the Year Leap?
     *
     * @param Year
     * @return true
     * @return false
     */
    bool InternalDoesLeap(int Year);

    /**
     * @brief Get the Localised Sun Rotation
     * Base Percents are the percent of a rotation around the globe
     * Location is player location from the base. This requires globe radius
     *
     * @param BaseLatitudePercent
     * @param BaseLongitudePercent
     * @param Location
     * @return FRotator
     */
    virtual FRotator GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);

    /**
     * @brief Get the Localised Moon Rotation
     * Base Percents are the percent of a rotation around the globe
     * Location is player location from the base. This requires globe radius
     *
     * @param BaseLatitudePercent
     * @param BaseLongitudePercent
     * @param Location
     * @return FRotator
     */
    virtual FRotator GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);

    ///// ///// ////////// ///// /////
    // Rollover Handlers
    //

    /**
     * @brief Roll over the days
     * Return true if the day rolled
     *
     * @param DateStruct
     * @return true
     * @return false
     */
    bool HandleDayRollover(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Roll over the months
     * Return true if the month rolled
     *
     * @param DateStruct
     * @return true
     * @return false
     */
    bool HandleMonthRollover(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Roll over the years
     * Return true if the year rolled
     *
     * @param DateStruct
     * @return true
     * @return false
     */
    bool HandleYearRollover(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Perform all rollovers
     * Return true if anything fired
     * -- The day would need updated
     *
     * @param DateStruct
     * @return true
     * @return false
     */
    bool SanitiseDateTime(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Perform solar day rollover
     * Return true if anything the day updated
     *
     * @param DateStruct
     * @return true
     * @return false
     */
    bool SanitiseSolarDateTime(FDateTimeSystemStruct &DateStruct);

public:
    /**
     * @brief Construct a new UDateTimeSystemComponent object
     */
    UDateTimeSystemComponent();

    /**
     * @brief Construct a new UDateTimeSystemComponent object
     */
    UDateTimeSystemComponent(UDateTimeSystemComponent &Other);

    /**
     * @brief Construct a new UDateTimeSystemComponent object
     */
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

    /**
     * @brief Add the DateStruct to current time
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    void AddDateStruct(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Advance to the timestamp specified in the DateStruct
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    void AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Advance to the timestamp specified in params
     * Returns true, unless the Safety failed due to the timestep being
     * less than 30 minutes
     *
     * @param Hour
     * @param Minute
     * @param Second
     * @param Safety
     * @return true
     * @return false
     */
    UFUNCTION(BlueprintCallable)
    bool AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety = true);

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     * Calls ComputeDeltaBetweenDatesYears
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    /**
     * @brief Compute Delta Between Dates in Fractional Months
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &From,
                                         UPARAM(ref) FDateTimeSystemStruct &To);

    /**
     * @brief Compute Delta Between Dates in Fractional Days
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To);

    /**
     * @brief Compute Delta Between Dates in Seconds
     * Not BP callable due to precision issues
     *
     * @param From
     * @param To
     * @return double
     */
    double DComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &From,
                                            UPARAM(ref) FDateTimeSystemStruct &To);

    /**
     * @brief Compute Delta Between Dates Internal
     *
     * @param From
     * @param To
     * @param OUT: Return the delta struct
     * @return TTuple<float, float, float>
     */
    TTuple<float, float, float> ComputeDeltaBetweenDatesInternal(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                 UPARAM(ref) FDateTimeSystemStruct &Date2,
                                                                 FDateTimeSystemStruct &Result);

    /**
     * @brief Get the Latitude From Location
     *
     * @param BaseLatitudePercent
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLatitudeFromLocation(float BaseLatitudePercent, FVector Location);

    /**
     * @brief Native Implementation of GetLatitudeFromLocation
     *
     * @param BaseLatitudePercent
     * @param Location
     * @return float
     */
    virtual float GetLatitudeFromLocation_Implementation(float BaseLatitudePercent, FVector Location);

    /**
     * @brief Get the Longitude From Location
     *
     * @param BaseLatitudePercent
     * @param BaseLongitudePercent
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLongitudeFromLocation(float BaseLatitudePercent, float BaseLongitudePercent, FVector Location);

    /**
     * @brief Native Implementation of GetLongitudeFromLocation
     *
     * @param BaseLatitudePercent
     * @param BaseLongitudePercent
     * @param Location
     * @return float
     */
    virtual float GetLongitudeFromLocation_Implementation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                          FVector Location);

    /**
     * @brief Does the Given Year Leap?
     *
     * @param Year
     * @return true
     * @return false
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    bool DoesYearLeap(int Year);

    /**
     * @brief Native Implementation of DoesYearLeap
     *
     * @param Year
     * @return true
     * @return false
     */
    virtual bool DoesYearLeap_Implementation(int Year);

    /**
     * @brief Get the Sun Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotationForLocation(FVector Location);

    /**
     * @brief Native Implementation for GetSunRotationForLocation
     *
     * @param Location
     * @return FRotator
     */
    virtual FRotator GetSunRotationForLocation_Implementation(FVector Location);

    /**
     * @brief Get the Sun Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotation();

    /**
     * @brief Native Implementation for GetSunRotation
     *
     * @return FRotator
     */
    virtual FRotator GetSunRotation_Implementation();

    /**
     * @brief Get the Sun Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FVector GetSunVector(float Latitude, float Longitude);

    /**
     * @brief Native Implementation for GetSunVector
     *
     * @return FVector
     */
    virtual FVector GetSunVector_Implementation(float Latitude, float Longitude);

    /**
     * @brief Get the Moon Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetMoonRotationForLocation(FVector Location);

    /**
     * @brief Native Implementation for GetMoonRotationForLocation
     *
     * @param Location
     * @return FRotator
     */
    virtual FRotator GetMoonRotationForLocation_Implementation(FVector Location);

    /**
     * @brief Get the Moon Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetMoonRotation();

    /**
     * @brief Native Implementation for GetMoonRotation
     *
     * @return FRotator
     */
    virtual FRotator GetMoonRotation_Implementation();

    /**
     * @brief Get the Moon Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FVector GetMoonVector(float Latitude, float Longitude);

    /**
     * @brief Native Implementation for GetMoonVector
     *
     * @return FVector
     */
    virtual FVector GetMoonVector_Implementation(float Latitude, float Longitude);

    ///// ///// ////////// ///// /////
    // Getters
    //

    /**
     * @brief Get the Fractional Days in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetFractionalDay(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Fractional Month in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetFractionalMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Fractional Orbital Year in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetFractionalOrbitalYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Fractional Calendar Days in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetFractionalCalendarYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief This function is used if the component cannot tick itself.
     * Such as, for example, when placed on a GameInstance.
     */
    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime, bool NonContiguous = false);

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

    /**
     * @brief Align the World Position to Date System Coordinate
     * By default, X is North.
     * If you wish to rotate this, pass the new north as a normalised vector
     *
     * @param WorldLocation
     * @param NorthingDirection
     * @return FVector
     */
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

    /**
     * @brief Get the Time Scale
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetTimeScale()
    {
        return TimeScale;
    }

    friend class UClimateComponent;
};
