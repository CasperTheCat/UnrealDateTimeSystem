// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

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

    UPROPERTY(EditAnywhere, Category = "Date and Time|Core")
    TSubclassOf<UDateTimeSystemCore> CoreClass;

    /**
     * @brief Length of a Day in Seconds
     * This refers to both Solar and Calendar days
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float LengthOfDay;

    /**
     * @brief Can This Tick?
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    bool CanEverTick;

    /**
     * @brief Time Scale
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float TimeScale;

    /**
     * @brief Time Scale
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float TicksPerSecond;

    /**
     * @brief Number of days, including any fractional component, in a year
     * Calendar Years are populated
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float DaysInOrbitalYear;

    /**
     * @brief Reference Latitude
     * All Latitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float ReferenceLatitude;

    /**
     * @brief Reference Longitude
     * All Longitude calculations are based from here
     *
     * Useful when using climate components aren't in use
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float ReferenceLongitude;

    /**
     * @brief Planet Radius in Kilometres
     * Used for computing sun and moon location at vector positions
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    float PlanetRadius;

    /**
     * @brief Number of Days in a week
     * Used for rolling over the day-of-week field
     *
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    int DaysInWeek;

    /**
     * @brief Yearbook
     * Uses the FDateTimeSystemYearbookRow row schema
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    TObjectPtr<UDataTable> YearBookTable;

    /**
     * @brief Overrides
     * Uses the FDateTimeSystemDateOverrideRow row schema
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
    TObjectPtr<UDataTable> DateOverridesTable;

    /**
     * @brief Whether set the overriden values when the date matches the current date
     * or the override dayindex matches the current dayindex
     *
     * Defaults to matching the date
     */
    UPROPERTY(EditAnywhere, Category = "Date and Time|Configuration")
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
    UPROPERTY(SaveGame, EditAnywhere, Category = "Date and Time|Configuration")
    FDateTimeSystemStruct InternalDate;

public:
    /**
     * @brief When an override occurs, do we want to update the date to the date in the override row?
     *
     * Note: This only makes sense when UseDayIndexForOverride is true,
     * which it is not by default.
     */
    UPROPERTY(SaveGame, EditAnywhere, Category = "Date and Time|Configuration")
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

    /**
     * @brief Called by BeginPlay
     * Can be called if the component isn't receiving a BeginPlay
     * Such as when on a GameInstance
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Begin")
    void InternalBegin();

    UFUNCTION(BlueprintCallable, Category = "Date and Time|Setters|TimeScale")
    void SetTimeScale(float NewScale);

    ///// ///// ////////// ///// /////
    // Core Functions
    //

    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Core")
    virtual UDateTimeSystemCore *GetCore() override;

    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Core")
    virtual bool IsReady() override;

    ///// ///// ////////// ///// /////
    // Date Functions
    //

    /**
     * @brief Populate DateStruct with today's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Today")
    virtual void GetTodaysDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with today's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Today")
    virtual void GetTodaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                 UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Populate DateStruct with tomorrow's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Tomorrow")
    virtual void GetTomorrowsDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with tomorrow's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Tomorrow")
    virtual void GetTomorrowsDateTZ(FDateTimeSystemStruct &DateStruct,
                                    UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Populate DateStruct with yesterday's date
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Yesterday")
    virtual void GetYesterdaysDate(FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Populate DateStruct with yesterday's date including Timezone Offset
     *
     * @param DateStruct
     * @param TimezoneInfo
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Yesterday")
    virtual void GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                     UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo) override;

    /**
     * @brief Set DTS date
     * Use SkipInitialisation if we are loading a checkpoint
     *
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Setters")
    virtual void SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation = false) override;

    /**
     * @brief Return a copy of the internal struct
     * Useful for saving the state
     *
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|UTCDate")
    virtual FDateTimeSystemStruct GetUTCDateTime() override;

    /**
     * Functions for Adding and Setting time in increments
     */

    /**
     * @brief Add the DateStruct to current time
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Add")
    virtual void AddDateStruct(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Advance to the timestamp specified in the DateStruct
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Set")
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
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Set")
    virtual bool AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety = true) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     * Calls ComputeDeltaBetweenDatesYears
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Delta")
    virtual float ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &From,
                                           UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Years
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Delta")
    virtual float ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &From,
                                                UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Months
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Delta")
    virtual float ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &From,
                                                 UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates in Fractional Days
     *
     * @param From
     * @param To
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Delta")
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
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Functions|Delta")
    virtual double ComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &From,
                                                   UPARAM(ref) FDateTimeSystemStruct &To) override;

    /**
     * @brief Compute Delta Between Dates Internal
     *
     * @param From
     * @param To
     * @param OUT: Return the delta struct
     *     1. DeltaYears
     *     2. DeltaMonths
     *     3. DeltaDays
     * @return TTuple<float, float, float>
     */
    virtual TTuple<float, float, float> ComputeDeltaBetweenDatesInternal(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                         UPARAM(ref) FDateTimeSystemStruct &Date2,
                                                                         FDateTimeSystemStruct &Result) override;

    ///// ///// ////////// ///// /////
    // Sun and Moon
    //

    /**
     * @brief Get the Sun Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Sun")
    virtual FRotator GetSunRotation() override;

    /**
     * @brief Get the Sun Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Sun")
    virtual FRotator GetSunRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Sun Rotation For Lat/Long
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Sun")
    virtual FRotator GetSunRotationForLatLong(double Latitude, double Longitude) override;

    /**
     * @brief Get the Sun Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Sun")
    virtual FVector GetSunVector(float Latitude, float Longitude) override;

    /**
     * @brief Get the Moon Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Moon")
    virtual FRotator GetMoonRotation() override;

    /**
     * @brief Get the Moon Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Moon")
    virtual FRotator GetMoonRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Moon Rotation For Lat/Long
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Moon")
    virtual FRotator GetMoonRotationForLatLong(double Latitude, double Longitude) override;

    /**
     * @brief Get the Moon Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Moon")
    virtual FVector GetMoonVector(float Latitude, float Longitude) override;

    /**
     * @brief Get the Moon Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|NightSky")
    virtual FMatrix GetNightSkyRotationMatrix() override;

    /**
     * @brief Get the Moon Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|NightSky")
    virtual FMatrix GetNightSkyRotationMatrixForLocation(FVector Location) override;

    /**
     * @brief Get the Moon Rotation For Lat/Long
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|NightSky")
    virtual FMatrix GetNightSkyRotationMatrixForLatLong(double Latitude, double Longitude) override;

    /**
     * @brief Get the Localised NightSky Rotation Matrix
     * Base Percents are the percent of a rotation around the globe
     * Location is player location from the base. This requires globe radius
     *
     * @param BaseLatitudePercent
     * @param BaseLongitudePercent
     * @param Location
     * @return FMatrix
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|NightSky")
    virtual FMatrix GetLocalisedNightSkyRotationMatrix(float BaseLatitudePercent, float BaseLongitudePercent,
                                                       FVector Location) override;

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
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Sun")
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
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Moon")
    virtual FRotator GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                              FVector Location) override;

    ///// ///// ////////// ///// /////
    // Misc Getters
    //

    /**
     * @brief Does the Given Year Leap?
     *
     * @param Year
     * @return true
     * @return false
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual bool DoesYearLeap(int Year) override;

    /**
     * @brief Get the Fractional Days in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Fractionals")
    virtual float GetFractionalDay(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Month in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Fractionals")
    virtual float GetFractionalMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Orbital Year in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Fractionals")
    virtual float GetFractionalOrbitalYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Fractional Calendar Days in DateStruct
     *
     * @param DateStruct
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Fractionals")
    virtual float GetFractionalCalendarYear(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Length Of a Calendar Year
     *
     * @param Year
     * @return int
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Fractionals")
    virtual int GetLengthOfCalendarYear(int Year) override;

    /**
     * @brief Get the number of days In current month
     *
     * @return int
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual int GetDaysInCurrentMonth() override;

    /**
     * @brief Get the number of days in the specified month by index
     *
     * @param MonthIndex
     * @return int
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual int GetDaysInMonth(int MonthIndex) override;

    /**
     * @brief Get the count of months In a year
     *
     * @param YearIndex
     * @return int
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual int GetMonthsInYear(int YearIndex) override;

    /**
     * @brief Return the FName of the month
     *
     * Remember, FName's are not localised, and this isn't using FText, which is
     * Using the FName directly in the UI is not advised
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual FText GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct) override;

    /**
     * @brief Get the Time Scale
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual float GetTimeScale() override;

    /**
     * @brief Get Length of Day
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Getters|Misc")
    virtual float GetLengthOfDay() override;

    ///// ///// ////////// ///// /////
    // Misc Helper Functions
    //

    /**
     * @brief This function is used if the component cannot tick itself.
     * Such as, for example, when placed on a GameInstance.
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Tick")
    virtual void InternalTick(float DeltaTime, bool NonContiguous = false) override;

    /**
     * @brief Align the World Position to Date System Coordinate
     * By default, X is North.
     * If you wish to rotate this, pass the new north as a normalised vector
     *
     * @param WorldLocation
     * @param NorthingDirection
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Alignment")
    virtual FVector AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection) override;

    /**
     * @brief Rotate the Location around world origin by the new north
     * By default, X is North.
     * If you wish to rotate this, pass the new north as a normalised vector
     *
     * @param WorldLocation
     * @param NorthingDirection
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Alignment")
    virtual FVector RotateLocationByNorthing(FVector Location, FVector NorthingDirection) override;

    /**
     * @brief Rotate the rotator around world origin by the new north
     * By default, X is North.
     * If you wish to rotate this, pass the new north as a normalised vector
     *
     * @param WorldLocation
     * @param NorthingDirection
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Alignment")
    virtual FRotator RotateRotationByNorthing(FRotator Rotation, FVector NorthingDirection) override;

    /**
     * @brief Combine the rotation around world origin by the new north
     * By default, X is North.
     * If you wish to rotate this, pass the new north as a normalised vector
     *
     * @param WorldLocation
     * @param NorthingDirection
     * @return FVector
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Alignment")
    virtual FMatrix RotateMatrixByNorthing(const FMatrix &RotationMatrix, FVector NorthingDirection) override;

    /**
     * @brief Sanitise Date Time
     *
     * @param FDateTimeSystemStruct
     * @return bool
     */
    UFUNCTION(BlueprintCallable, Category = "Date and Time|Internal|Sanitise")
    virtual bool SanitiseDateTime(FDateTimeSystemStruct &DateStruct) override;

    virtual void RegisterForNotification(TScriptInterface<IDateTimeNotifyInterface> Interface) override;
    virtual void UnregisterForNotification(TScriptInterface<IDateTimeNotifyInterface> Interface) override;

    friend class UClimateComponent;
};
