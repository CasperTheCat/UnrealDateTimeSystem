// [TEMPLATE_COPYRIGHT]

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "DateTimeSystemDataRows.h"
#include "DateTimeTypes.h"
#include "GameplayTagContainer.h"
#include "Interfaces.h"

#include "DateTimeSubsystem.generated.h"

// DATETIMESYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogDateTimeSystem, Log, All);
DECLARE_STATS_GROUP(TEXT("DateTimeSubsystem"), STATGROUP_ACIDateTimeSubsys, STATCAT_Advanced);

// Forward Decl
class UClimateComponent;

/**
 * @brief DateTimeSubsystem
 *
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent),
       DisplayName = "Date Time System")
class DATETIMESYSTEM_API UDateTimeSystem
    : public UGameInstanceSubsystem
    , public FTickableGameObject
    , public IDateTimeSystemCommon
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TObjectPtr<UDateTimeSystemCore> CoreObject;

    /**
     *
     */
    UPROPERTY()
    float StoredDeltaTime;

    /**
     * @brief Sets how many times per second this object should tick
     * Only used if BeginPlay is called by the engine
     *
     * Set to Zero (0) to use default
     */
    UPROPERTY()
    int CurrentTickIndex;

    /**
     * @brief Length of a year in calendar days
     *
     */
    UPROPERTY(Transient)
    int LengthOfCalendarYearInDays;

public:
    /**
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystem();

    /**
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystem(UDateTimeSystem &Other);

    /**
     * @brief Construct a new UDateTimeSystem object
     */
    UDateTimeSystem(const FObjectInitializer &ObjectInitializer);

    
    ///// ///// ////////// ///// /////
    // Core Functions
    //

    UFUNCTION(BlueprintCallable)
    virtual UDateTimeSystemCore *GetCore() override;

    UFUNCTION(BlueprintCallable)
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
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetSunRotation() override;

    /**
     * @brief Get the Sun Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetSunRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Sun Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable)
    virtual FVector GetSunVector(float Latitude, float Longitude) override;

    /**
     * @brief Get the Moon Rotation
     *
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetMoonRotation() override;

    /**
     * @brief Get the Moon Rotation For WorldLocation
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    virtual FRotator GetMoonRotationForLocation(FVector Location) override;

    /**
     * @brief Get the Moon Vector
     *
     * @return FVector
     */
    UFUNCTION(BlueprintCallable)
    virtual FVector GetMoonVector(float Latitude, float Longitude) override;

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
    UFUNCTION(BlueprintCallable)
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
    UFUNCTION(BlueprintCallable)
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
    UFUNCTION(BlueprintCallable)
    virtual bool DoesYearLeap(int Year) override;

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
     * @brief Get the Length Of a Calendar Year
     *
     * @param Year
     * @return int
     */
    UFUNCTION(BlueprintCallable)
    virtual int GetLengthOfCalendarYear(int Year) override;

    /**
     * @brief Get the number of days In current month
     *
     * @return int
     */
    UFUNCTION(BlueprintCallable)
    virtual int GetDaysInCurrentMonth() override;

    /**
     * @brief Get the number of days in the specified month by index
     *
     * @param MonthIndex
     * @return int
     */
    UFUNCTION(BlueprintCallable)
    virtual int GetDaysInMonth(int MonthIndex) override;

    /**
     * @brief Get the count of months In a year
     *
     * @param YearIndex
     * @return int
     */
    UFUNCTION(BlueprintCallable)
    virtual int GetMonthsInYear(int YearIndex) override;

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

    /**
     * @brief Get Length of Day
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    virtual float GetLengthOfDay() override;

    ///// ///// ////////// ///// /////
    // Misc Helper Functions
    //

    /**
     * @brief This function is used if the component cannot tick itself.
     * Such as, for example, when placed on a GameInstance.
     */
    UFUNCTION(BlueprintCallable)
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
    UFUNCTION(BlueprintCallable)
    virtual FVector AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection) override;

    /**
     * @brief Sanitise Date Time
     *
     * @param FDateTimeSystemStruct
     * @return bool
     */
    UFUNCTION(BlueprintCallable)
    virtual bool SanitiseDateTime(FDateTimeSystemStruct &DateStruct) override;



    friend class UClimateComponent;

public:
    //~USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject *Outer) const override;
    //~End of USubsystem interface

    //~FTickableObjectBase interface
    virtual void Tick(float DeltaTime) override;
    virtual ETickableTickType GetTickableTickType() const override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    virtual UWorld *GetTickableGameObjectWorld() const override;
    //~End of FTickableObjectBase interface
};
