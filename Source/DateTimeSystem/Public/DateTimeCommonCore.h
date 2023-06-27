// [TEMPLATE_COPYRIGHT]

#pragma once

#include "CoreMinimal.h"
#include "DateTimeSystemDataRows.h"
#include "DateTimeTypes.h"
#include "GameplayTagContainer.h"

#include "DateTimeCommonCore.generated.h"

// DATETIMESYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogDateTimeSystem, Log, All);
DECLARE_STATS_GROUP(TEXT("DateTimeSubsystem"), STATGROUP_ACIDateTimeCommon, STATCAT_Advanced);

// Forward Decl
class UClimateComponent;

/**
 * @brief DateTimeSubsystem
 *
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), DisplayName = "Date Time System")
class DATETIMESYSTEM_API UDateTimeSystemCore : public UObject
{
    GENERATED_BODY()

private:
    /**
     * @brief Length of a Day in Seconds
     * This refers to both Solar and Calendar days
     */
    UPROPERTY()
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
    UPROPERTY()
    int CurrentTickIndex;

    /**
     * @brief Latitude as a percentage
     * For ease of use internally
     */
    UPROPERTY()
    float PercentLatitude;

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
    UPROPERTY()
    float PlanetRadius;

    /**
     * @brief Runtime Reciprocal for Planet Radius
     */
    UPROPERTY()
    double InvPlanetRadius;

    /**
     * @brief Whether set the overriden values when the date matches the current date
     * or the override dayindex matches the current dayindex
     *
     * Defaults to matching the date
     */
    UPROPERTY()
    bool UseDayIndexForOverride;

    /**
     * @brief When an override occurs, do we want to update the date to the date in the override row?
     *
     * Note: This only makes sense when UseDayIndexForOverride is true,
     * which it is not by default.
     */
    UPROPERTY()
    bool OverridedDatesSetDate;

    /**
     * @brief Number of days, including any fractional component, in a year
     * Calendar Years are populated
     */
    UPROPERTY()
    float DaysInOrbitalYear;

    /**
     * @brief Number of Days in a week
     * Used for rolling over the day-of-week field
     *
     */
    UPROPERTY()
    int DaysInWeek;

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
    UPROPERTY(EditAnywhere)
    FDateTimeSystemStruct InternalDate;

public:
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
     */
    UPROPERTY(BlueprintAssignable)
    FInvalidationDelegate InvalidationCallback;

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

public:
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
     * @brief Get the Julian Day
     *
     * @param DateStruct
     * @return double
     */
    double GetSolarYears(FDateTimeSystemStruct &DateStruct);

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
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystemCore();

    /**
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystemCore(UDateTimeSystemCore &Other);

    /**
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystemCore(const FObjectInitializer &ObjectInitializer);

    ///**
    // * @brief Engine tick function. Called when this component is on a tickable actor.
    // * If it isn't, call InternalTick manually.
    // *
    // * @param DeltaTime
    // * @param TickType
    // * @param ThisTickFunction
    // */
    // virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
    //                           FActorComponentTickFunction *ThisTickFunction) override;

    ///**
    // * @brief Engine Begin Function. Called by the engine on a tickable actor.
    // * If it isn't, call InternalBegin manually.
    // */
    // virtual void BeginPlay();

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
    void InternalBegin(FDateTimeCommonCoreInitializer &CoreInitializer);

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

    UFUNCTION(BlueprintCallable)
    float GetLengthOfDay();

    friend class UClimateComponent;
};
