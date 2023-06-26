// [TEMPLATE_COPYRIGHT]

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "DateTimeCommonCore.h"
#include "DateTimeSystemDataRows.h"
#include "DateTimeTypes.h"
#include "GameplayTagContainer.h"
#include "Interfaces.h"

#include "DateTimeSystemComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("DateTimeSystem"), STATGROUP_ACIDateTimeSys, STATCAT_Advanced);

// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCleanDateChangeDelegate);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDateChangeDelegate, FDateTimeSystemStruct, NewDate);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOverridesDelegate, FDateTimeSystemStruct, NewDate,
// FGameplayTagContainer,
//                                              Attribute);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvalidationDelegate, EDateTimeSystemInvalidationTypes,
// InvalidationType);

// Forward Decl
class UClimateComponent;

/**
 * @brief DateTimeSystemComponent
 *
 * Subclasses UActorComponent so it can be placed in more locations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent),
       DisplayName = "Date Time System")
class DATETIMESYSTEM_API UDateTimeSystemComponent
    : public UActorComponent
    , public IDateTimeSystemCommon
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TObjectPtr<UDateTimeSystemCore> CoreObject;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UDateTimeSystemCore> CoreClass;

    /**
     * @brief Length of a Day in Seconds
     * This refers to both Solar and Calendar days
     */
    UPROPERTY(EditAnywhere)
    float LengthOfDay;

    /**
     * @brief Can This Tick?
     */
    UPROPERTY(EditAnywhere)
    bool CanEverTick;

    /**
     * @brief Time Scale
     */
    UPROPERTY(EditAnywhere)
    float TimeScale;

    /**
     * @brief Time Scale
     */
    UPROPERTY(EditAnywhere)
    float TicksPerSecond;

    /**
     * @brief Number of days, including any fractional component, in a year
     * Calendar Years are populated
     */
    UPROPERTY(EditAnywhere)
    float DaysInOrbitalYear;

    /**
     * @brief Reference Latitude
     * All Latitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditAnywhere)
    float ReferenceLatitude;

    /**
     * @brief Reference Longitude
     * All Longitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditAnywhere)
    float ReferenceLongitude;

    /**
     * @brief Planet Radius in Kilometres
     * Used for computing sun and moon location at vector positions
     */
    UPROPERTY(EditAnywhere)
    float PlanetRadius;

    /**
     * @brief Number of Days in a week
     * Used for rolling over the day-of-week field
     *
     */
    UPROPERTY(EditAnywhere)
    int DaysInWeek;

    /**
     * @brief Yearbook
     * Uses the FDateTimeSystemYearbookRow row schema
     */
    UPROPERTY(EditAnywhere)
    TObjectPtr<UDataTable> YearBookTable;

    /**
     * @brief Overrides
     * Uses the FDateTimeSystemDateOverrideRow row schema
     */
    UPROPERTY(EditAnywhere)
    TObjectPtr<UDataTable> DateOverridesTable;

    /**
     * @brief Whether set the overriden values when the date matches the current date
     * or the override dayindex matches the current dayindex
     *
     * Defaults to matching the date
     */
    UPROPERTY(EditAnywhere)
    bool UseDayIndexForOverride;

    /**
     * @brief Length of a year in calendar days
     *
     */
    UPROPERTY(Transient)
    int LengthOfCalendarYearInDays;

    /**
     * @brief Internal Date and Time stored in UTC
     *
     */
    UPROPERTY(SaveGame, EditAnywhere)
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

private:
    ///**
    // * @brief Get the Hash For Date object
    // *
    // * @param DateStruct
    // * @return uint32
    // */
    // uint32 GetHashForDate(FDateTimeSystemStruct *DateStruct);

    ///**
    // * @brief Get the Hash For Date object
    // *
    // * @param DateStruct
    // * @return uint32
    // */
    // uint32 GetHashForDate(FDateTimeSystemDateOverrideRow *DateStruct);

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
    virtual int GetDaysInCurrentMonth() override;

    /**
     * @brief Get the number of days in the specified month by index
     *
     * @param MonthIndex
     * @return int
     */
    virtual int GetDaysInMonth(int MonthIndex) override;

    /**
     * @brief Get the count of months In a year
     *
     * @param YearIndex
     * @return int
     */
    virtual int GetMonthsInYear(int YearIndex) override;

    /**
     * @brief Get the Length Of a Calendar Year
     *
     * @param Year
     * @return int
     */
    virtual int GetLengthOfCalendarYear(int Year) override;

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
    virtual FRotator GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                             FVector Location) override;

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
    virtual FRotator GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                              FVector Location) override;

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
    virtual void BeginPlay() override;

    virtual bool IsReady() override;

    ///// ///// ////////// ///// /////
    // Date Functions
    //

    /**
     * @brief Populate DateStruct with today's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetTodaysDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with today's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetTodaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                 UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Populate DateStruct with tomorrow's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetTomorrowsDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with tomorrow's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetTomorrowsDateTZ(FDateTimeSystemStruct &DateStruct,
                                    UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Populate DateStruct with yesterday's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetYesterdaysDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with yesterday's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable)
    virtual void GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                     UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Set DTS date
     * Use SkipInitialisation if we are loading a checkpoint
     *
     */
    UFUNCTION(BlueprintCallable)
    virtual void SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation = false) override;

    /**
     * @brief Return a copy of the internal struct
     * Useful for saving the state
     *
     */
    UFUNCTION(BlueprintCallable)
    virtual FDateTimeSystemStruct GetUTCDateTime() override;

    /**
     * Functions for Adding and Setting time in increments
     */

    /**
     * @brief Add the DateStruct to current time
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    virtual void AddDateStruct(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Advance to the timestamp specified in the DateStruct
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable)
    virtual void AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

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
    virtual bool AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety = true) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     * Calls ComputeDeltaBetweenDatesYears
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &From,
                                           UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &From,
                                                UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Months
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &From,
                                                 UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Days
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &From,
                                               UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Seconds
     * Not BP callable due to precision issues
     *
     * @param From
     * @param To
     * @return double
     */
    virtual double DComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &From,
                                                    UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates Internal
     *
     * @param From
     * @param To
     * @param OUT: Return the delta struct
     * @return TTuple<float, float, float>
     */
    virtual TTuple<float, float, float> ComputeDeltaBetweenDatesInternal(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                         UPARAM(ref) FDateTimeSystemStruct &Date2,
                                                                         FDateTimeSystemStruct &Result) override;

    /**
     * @brief Get the Sun Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetSunRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Sun Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetSunRotation() override;

    /**
     * @brief Get the Sun Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable)
    virtual FVector GetSunVector(float Latitude, float Longitude) override;

    /**
     * @brief Get the Moon Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetMoonRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Moon Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetMoonRotation() override;

    /**
     * @brief Get the Moon Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable)
    virtual FVector GetMoonVector(float Latitude, float Longitude) override;

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
    virtual float GetFractionalDay(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Month in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float GetFractionalMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Orbital Year in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float GetFractionalOrbitalYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Calendar Days in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float GetFractionalCalendarYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief This function is used if the component cannot tick itself.
     * Such as, for example, when placed on a GameInstance.
     */
    UFUNCTION(BlueprintCallable)
    virtual void InternalTick(float DeltaTime, bool NonContiguous = false) override;

    /**
     * @brief Called by BeginPlay
     * Can be called if the component isn't receiving a BeginPlay
     * Such as when on a GameInstance
     */
    UFUNCTION(BlueprintCallable)
    void InternalBegin();

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
    virtual FVector AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection) override;

    /**
     * @brief Return the FName of the month
     *
     * Remember, FName's are not localised, and this isn't using FText, which is
     * Using the FName directly in the UI is not advised
     */
    UFUNCTION(BlueprintCallable)
    virtual FText GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Time Scale
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float GetTimeScale() override;

    UFUNCTION(BlueprintCallable)
    virtual float GetLengthOfDay() override;

    /**
     * @brief Set the Time Scale
     */
    UFUNCTION(BlueprintCallable)
    void SetTimeScale(float NewScale);

    virtual UDateTimeSystemCore *GetCore() override;

    friend class UClimateComponent;
};
