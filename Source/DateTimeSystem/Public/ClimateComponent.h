// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "DateTimeSystem/Public/DateTimeTypes.h"
#include "DateTimeSystemComponent.h"
#include "DateTimeSystemDataRows.h"
#include "GameplayTagContainer.h"
#include "Interfaces.h"

#include "ClimateComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("ClimateSystem"), STATGROUP_ACIClimateSys, STATCAT_Advanced);

USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeClimateDataStruct
{
    GENERATED_BODY()

public:
    // Encodes how much water there is in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Wetness;

    // Amount of Rain to apply to shader
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Rain;

    // Accumulated Puddles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Puddles;

    // WindVector
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector Wind;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float ChillOffset;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float HeatOffset;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangeDelegate, float, NewTemperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateClimateData, FDateTimeClimateDataStruct, ClimateData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDTSUpdateTime, FDateTimeSystemStruct, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLocalDateTimeEvent);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent),
       DisplayName = "DTS Climate Subsystem")
class DATETIMESYSTEM_API UClimateComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    /**
     * @brief Tick rate when engine is ticking the component
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float TicksPerSecond;

    /**
     * @brief Multiplier for rain change
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float RainProbabilityMultiplier;

    /**
     * @brief Threshold at which to use a non-contiguous time update
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float CatchupThresholdInSeconds;

    /**
     * @brief How fast should Wetness catch up to the target when stopping raining
     * Only used by non-contiguous updates
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float CatchupWetnessDownblendSpeed;

    /**
     * @brief How fast should Wetness catch up to the target when starting raining
     * Only used by non-contiguous updates
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float CatchupWetnessUpblendSpeed;

    /**
     * @brief How fast should Wetness catch up to the target when starting raining
     * Only used by non-contiguous updates
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float CatchupSittingWaterLimit;

    /**
     * @brief Percentage of current Wetness that should evaporate over the next minute
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float WetnessEvaporationRate;

    /**
     * @brief Rate at which rainfall creates wetness
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float WetnessDepositionRate;

    /**
     * @brief Base percentage of current Wetness that should evaporate over the next minute
     * Used when the sun is set or at night local time depending on UseSunPositionForEvaporation
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float WetnessEvaporationRateBase;

    /**
     * @brief Whether to use the sun's position for evaporation
     * Fallback is a linear interpolate over the day, which removes seasonality
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    bool UseSunPositionForEvaporation;

    /**
     * @brief Speed that rainfall lerps up to target in live update
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float RainfallBlendIncreaseSpeed;

    /**
     * @brief Speed that rainfall lerps down to target in live update
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float RainfallBlendDecreaseSpeed;

    /**
     * @brief Fraction of the Wetness that spills over into puddles over 1.f
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float RainfallWetnessOverflowPuddlingScale;

    /**
     * @brief Percentage of current puddles that should evaporate over the next minute
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float PuddleEvaporationRate;

    /**
     * @brief Base percentage of current puddles that should evaporate over the next minute
     * Used when the sun is set or at night local time depending on UseSunPositionForEvaporation
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float PuddleEvaporationRateBase;

    /**
     * @brief Upper Limit to stored water
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float PuddleLimit;

    /**
     * @brief Climate Data Table
     * Uses FDateTimeSystemClimateMonthlyRow
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    TObjectPtr<UDataTable> ClimateTable;

    /**
     * @brief Climate Override Table
     * Uses FDateTimeSystemClimateOverrideRow
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    TObjectPtr<UDataTable> ClimateOverridesTable;

    /**
     * @brief Climate Data Override Array
     *
     */
    TArray<FDateTimeSystemClimateOverrideRow *> DOTemps;

    /**
     * @brief Map for looking up Overrides
     *
     */
    TMap<uint32, FDateTimeSystemClimateOverrideRow *> DateOverrides;

    /**
     * @brief Climate Data
     *
     */
    TArray<FDateTimeSystemClimateMonthlyRow *> ClimateBook;

    /**
     * @brief Date Time System
     * Runtime, may be nullptr
     *
     */
    UPROPERTY(Transient)
    TScriptInterface<IDateTimeSystemCommon> DateTimeSystem;

    /**
     * @brief HourlyToBinly
     *
     */
    UPROPERTY(Transient)
    float HourlyToPerBin;

    /**
     * @brief Previous High Temp for blending
     *
     */
    UPROPERTY(Transient)
    float LastHighTemp;

    /**
     * @brief Previous Low Temp for blending
     *
     */
    UPROPERTY(Transient)
    float LastLowTemp;

    /**
     * @brief Latitude as a percentage of a rotation
     *
     */
    UPROPERTY(Transient)
    float PercentileLatitude;

    /**
     * @brief Longitude as a percentage of a rotation
     *
     */
    UPROPERTY(Transient)
    float PercentileLongitude;

    /**
     * @brief Latitude in radians
     *
     */
    UPROPERTY(Transient)
    float RadLatitude;

    /**
     * @brief Longitude in radians
     *
     */
    UPROPERTY(Transient)
    float RadLongitude;

    /**
     * @brief Reciprocal climate Update Frequency
     * Not the same as tick rate
     *
     */
    UPROPERTY(SaveGame)
    float OneOverUpdateFrequency;

    /**
     * @brief Stored time since last update
     *
     */
    UPROPERTY(Transient)
    float AccumulatedDeltaForCallback;

    /**
     * @brief Boolean Flag for sun being above the horizon
     *
     */
    UPROPERTY(SaveGame)
    bool SunHasRisen;

    /**
     * @brief Boolean Flag for sun being below the horizon
     *
     */
    UPROPERTY(SaveGame)
    bool SunHasSet;

    /**
     * @brief Cached Daily High Temp
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedHighTemp;

    /**
     * @brief Cached Daily Low Temp
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedLowTemp;

    /**
     * @brief Cached Daily Low for the prior day
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedNextLowTemp;

    /**
     * @brief Cached Prior dew point
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedPriorDewPoint;

    /**
     * @brief Cached next dew point
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedNextDewPoint;

    /**
     * @brief Cached Prior rainfall
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedPriorRainfall;

    /**
     * @brief Cached next rainfall
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedNextRainfall;

    /**
     * @brief Cached analytic dew point
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedAnalyticalDewPoint;

    /**
     * @brief Cached analytic monthly high temperature
     * This is the blended temperature evaluated every day
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyHighTemp;

    /**
     * @brief Cached analytic monthly low temperature
     * This is the blended temperature evaluated every day
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyLowTemp;

    /**
     * @brief Cached Probability of Precipitation
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, float> CachedProbability;

    /**
     * @brief Cached Analytic Probability of Precipitation
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, float> CachedAnalyticProbability;

    /**
     * @brief Cached Probability of Precipitation
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, float> CachedRainfallLevels;

    /**
     * @brief Cached Analytic Probability of Precipitation
     *
     */
    UPROPERTY(Transient)
    TMap<uint32, float> CachedAnalyticRainfallLevel;

    /**
     * @brief Local Time Post Update
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemStruct LocalTime;

    /**
     * @brief Local Time Post Update
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemStruct PriorLocalTime;

public:
    /**
     * @brief Temperature Change Callback
     *
     */
    UPROPERTY(BlueprintAssignable)
    FTemperatureChangeDelegate TemperatureChangeCallback;

    /**
     * @brief Temperature Change Callback
     * Try to avoid this call if you can
     */
    UPROPERTY(BlueprintAssignable)
    FUpdateClimateData UpdateLocalClimateCallback;

    /**
     * @brief Temperature Change Callback
     * Lightweight call. Use functions to read the data
     */
    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent UpdateLocalClimateSignal;

    /**
     * @brief Temperature Change Callback
     * Try to avoid this call if you can
     */
    UPROPERTY(BlueprintAssignable)
    FInvalidationDelegate InvalidationCallback;

    /**
     * @brief DateTime Update Callback
     *
     */
    UPROPERTY(BlueprintAssignable)
    FDTSUpdateTime UpdateLocalTime;

    /**
     * @brief Lighter weight DateTime Update Callback
     *
     */
    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent LocalTimeUpdateSignal;

    /**
     * @brief Callback for when the sun rises
     *
     */
    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunriseCallback;

    /**
     * @brief Callback for when the sun sets
     *
     */
    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunsetCallback;

    /**
     * @brief Callback for when the sun is below the horizon, but still lighting
     *
     */
    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent TwilightCallback;

    /**
     * @brief Computed Temperature
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentTemperature;

    /**
     * @brief Computed Rainfall
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentRainfall;

    /**
     * @brief Computed Wetness
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentWetness;

    /**
     * @brief Computed Wetness Limit
     * Used to clamp
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentWetnessLimit;

    /**
     * @brief Computed Sitting Water
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentSittingWater;

    /**
     * @brief Computed Sitting Water
     * Used to clamp
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentSittingWaterLimit;

    /**
     * @brief Computed Humidity
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentRelativeHumidity;

    /**
     * @brief Computed Dew Point
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentDewPoint;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    int NumberOfRainSlotsPerDay;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentPrecipitationLevel;

    /**
     * @brief Computed Fog Level
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|State")
    float CurrentFog;

    /**
     * @brief Temperature Update Rate
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    float TemperatureChangeSpeed;

    /**
     * @brief Fog Update Rate
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    float FogChangeSpeed;

    /**
     * @brief Mean Sea Level
     * Used for offsetting the floor
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    float SeaLevel;

    /**
     * @brief Have we bound to the DateTimeSystem?
     *
     */
    UPROPERTY(Transient)
    bool HasBoundToDate;

    /**
     * @brief DateTime Timescale
     *
     */
    UPROPERTY(Transient)
    float DTSTimeScale;

    /**
     * @brief At what angle below the horizon, in radians, does sunset callback trigger
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    float SunPositionBelowHorizonThreshold;

    /**
     * @brief At what angle above the horizon, in radians, does sunrise callback trigger
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    float SunPositionAboveHorizonThreshold;

    /**
     * @brief Base Latitude relative to the DTS's reference
     *
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate|Internal|Configuration")
    float ReferenceLatitude;

    /**
     * @brief Base Longitude relative to the DTS's reference
     *
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climate|Internal|Configuration")
    float ReferenceLongitude;

    /**
     * @brief Default Climate Update Rate
     * Use SetClimateUpdateFrequency to set in gameplay
     *
     */
    UPROPERTY(EditAnywhere, Category = "Climate|Internal|Configuration")
    float DefaultClimateUpdateFrequency;

    /**
     * @brief Timezone for this Climate Region
     * Does not support DST as that's hard
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    FDateTimeSystemTimezoneStruct TimezoneInfo;

    /**
     * @brief Direction of North Vector
     * Used to rotate Vectors
     * Defaults to +X North
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Climate|Internal|Configuration")
    FVector NorthingDirection;

private:
    /**
     * @brief Called by constructors
     *
     */
    void ClimateSetup();

    /**
     * @brief Invalidation
     *
     */
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

    /**
     * @brief Passthrough Function for LocalTime update
     *
     */
    UFUNCTION()
    void UpdateLocalTimePassthrough();

private:
    /**
     * @brief Get the Analytical High For DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetAnalyticalHighForDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Analytical Low For DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetAnalyticalLowForDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Analytical Dew Point For DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetAnalyticalDewPointForDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Daily High for DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetDailyHigh(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Daily Low for DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetDailyLow(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the precipitation probability for DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetPrecipitationThreshold(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Analytical precipitation probability For DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetAnalyticalPrecipitationThresholdDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the precipitation probability for DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetRainfallAmount(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Analytical precipitation probability For DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetAnalyticalPrecipitationAmountDate(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Get the Daily Dew Point for DateStruct
     *
     * @param DateStruct
     * @return float
     */
    float GetDailyDewPoint(FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Update the current Temperature
     *
     * @param DeltaTime
     */
    void UpdateCurrentTemperature(float DeltaTime, bool NonContiguous);

    /**
     * @brief Update the current Temperature
     *
     * @param DeltaTime
     */
    void UpdateCurrentRainfall(float DeltaTime, bool NonContiguous);

    /**
     * @brief Update the Current Climate
     *
     * @param DeltaTime
     */
    void UpdateCurrentClimate(float DeltaTime, bool NonContiguous);

    /**
     * @brief Called by DTS via callback
     * Dynamic doesn't like reference
     *
     */
    UFUNCTION()
    void InternalDateChanged(FDateTimeSystemStruct DateStruct);

    /**
     * @brief Find DTS
     *
     */
    TScriptInterface<IDateTimeSystemCommon> FindComponent();

    /**
     * @brief Internal function for adjusting Location Northing Vector
     *
     */
    FVector GetLocationAdjustedForNorthing(FVector Location);

    /**
     * @brief Internal function for adjusting Location Northing Vector
     *
     */
    FVector RotateByNorthing(FVector Location);

    /**
     * @brief Internal function for adjusting Location Northing Vector
     *
     */
    FRotator RotateByNorthing(FRotator Rotation);

public:
    /**
     * @brief Construct a new UClimateComponent
     *
     */
    UClimateComponent();

    /**
     * @brief Construct a new UClimateComponent
     *
     * @param Other
     */
    UClimateComponent(UClimateComponent &Other);

    /**
     * @brief Construct a new UClimateComponent
     *
     * @param ObjectInitializer
     */
    UClimateComponent(const FObjectInitializer &ObjectInitializer);

    /**
     * @brief Engine Tick Function
     *
     * @param DeltaTime
     * @param TickType
     * @param ThisTickFunction
     */
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    /**
     * @brief Engine Begin Play
     *
     */
    virtual void BeginPlay() override;

    /**
     * @brief Get the Local Time.
     * Warning! This function may be off by up to one frame if GetLocalTime
     * Is called before InternalTick is.
     *
     * @return FDateTimeSystemStruct
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Time")
    FDateTimeSystemStruct GetLocalTime();

    /**
     * @brief Bind to the DTS
     * Normally called by InternalBegin
     *
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Internal|Bind")
    void BindToDateTimeSystem();

    /**
     * @brief Return whether the sun has risen
     *
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Sun")
    bool GetHasSunRisen()
    {
        return SunHasRisen;
    }

    /**
     * @brief Return whether the sun has set
     *
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Sun")
    bool GetHasSunSet()
    {
        return SunHasSet;
    }

    /**
     * @brief Get the Monthly High Temperature
     *
     * @param MonthIndex
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetMonthlyHighTemperature(int MonthIndex);

    /**
     * @brief Get the Monthly Low Temperature
     *
     * @param MonthIndex
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetMonthlyLowTemperature(int MonthIndex);

    /**
     * @brief Get the Current Temperature
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetCurrentTemperature();

    /**
     * @brief Get the Current Rainfall
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Rainfall")
    float GetCurrentRainfall();

    /**
     * @brief Get the Current Wetness
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Wetness")
    float GetCurrentWetness();

    /**
     * @brief Get the Current Sitting Overflow
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Wetness")
    float GetCurrentSittingWater();

    // TODO: DELETE
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Wetness")
    float DebugGetUnclampedWetness();

    /**
     * @brief Get the Current Temperature For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetCurrentTemperatureForLocation(FVector Location);

    /**
     * @brief Get the Current Felt Temperature
     *
     * @param WindVelocity
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetCurrentFeltTemperature(float WindVelocity);

    /**
     * @brief Get the Current Felt Temperature For Location
     *
     * @param WindVelocity
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetCurrentFeltTemperatureForLocation(float WindVelocity, FVector Location);

    /**
     * @brief Get the Current Dew Point For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Dew Point")
    float GetCurrentDewPointForLocation(FVector Location);

    /**
     * @brief Get the Relative Humidity For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Humidity")
    float GetRelativeHumidityForLocation(FVector Location);

    /**
     * @brief Get the Cloud Level
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Cloud")
    float GetCloudLevel();

    /**
     * @brief Get the Fog Level
     *
     * @param DeltaTime
     * @param Scale
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Fog")
    float GetFogLevel(float DeltaTime, float Scale = 1.f);

    /**
     * @brief Get the Heat Index
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetHeatIndex();

    /**
     * @brief Get the Heat Index For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetHeatIndexForLocation(FVector Location);

    /**
     * @brief Get the Wind Chill From Vector
     *
     * @param WindVector
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetWindChillFromVector(FVector WindVector);

    /**
     * @brief Get the Wind Chill From Velocity
     *
     * @param WindVelocity
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Temperature")
    float GetWindChillFromVelocity(float WindVelocity);

    /**
     * @brief Internal Tick
     * Call this is the engine isn't ticking the component
     * For example, when it is on a GameInstance
     *
     * @param DeltaTime
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Internal|Tick")
    void InternalTick(float DeltaTime);

    /**
     * @brief Called by BeginPlay
     * Can be called if the component isn't receiving a BeginPlay
     * Such as when on a GameInstance
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Internal|Begin")
    void InternalBegin();

    /**
     * @brief Set the Climate Update Frequency
     *
     * @param Frequency
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Setters|Update")
    void SetClimateUpdateFrequency(float Frequency);

    /**
     * @brief Get the Local Sun Rotation object
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Sun")
    FRotator GetLocalSunRotation(FVector Location);

    /**
     * @brief Get the Local Moon Rotation object
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Getters|Moon")
    FRotator GetLocalMoonRotation(FVector Location);

    /**
     * @brief Called internally when the date changed
     * No default implementation, override this to add features
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Date")
    void DateChanged(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Native Implementation for DateChanged
     *
     * @param DateStruct
     */
    virtual void DateChanged_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    /**
     * @brief Modulate the Daily Low
     *
     * @param DateStruct
     * @param Attributes
     * @param Temperature
     * @param PreviousDayLow
     * @param PreviousDayHigh
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Temperature")
    float DailyLowModulation(UPARAM(ref) FDateTimeSystemStruct &DateStruct, FGameplayTagContainer &Attributes,
                             float Temperature, float PreviousDayLow, float PreviousDayHigh);

    /**
     * @brief Native Implementation for DailyLowModulation
     *
     * @param DateStruct
     * @param Attributes
     * @param Temperature
     * @param PreviousDayLow
     * @param PreviousDayHigh
     * @return float
     */
    virtual float DailyLowModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                    FGameplayTagContainer &Attributes, float Temperature,
                                                    float PreviousDayLow, float PreviousDayHigh);

    /**
     * @brief Modulate the Daily High
     *
     * @param DateStruct
     * @param Attributes
     * @param Temperature
     * @param PreviousDayLow
     * @param PreviousDayHigh
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Temperature")
    float DailyHighModulation(UPARAM(ref) FDateTimeSystemStruct &DateStruct, FGameplayTagContainer &Attributes,
                              float Temperature, float PreviousDayLow, float PreviousDayHigh);

    /**
     * @brief Native Implementation for DailyHighModulation
     *
     * @param DateStruct
     * @param Attributes
     * @param Temperature
     * @param PreviousDayLow
     * @param PreviousDayHigh
     * @return float
     */
    virtual float DailyHighModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                     FGameplayTagContainer &Attributes, float Temperature,
                                                     float PreviousDayLow, float PreviousDayHigh);

    /**
     * @brief Called to modulate the temps over the day
     *
     * @param Temperature
     * @param SecondsSinceUpdate
     * @param LowTemperature
     * @param HighTemperature
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Temperature")
    float ModulateTemperature(float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature);

    /**
     * @brief Native Implementation of ModulateTemperature
     *
     * @param Temperature
     * @param SecondsSinceUpdate
     * @param LowTemperature
     * @param HighTemperature
     * @return float
     */
    virtual float ModulateTemperature_Implementation(float Temperature, float SecondsSinceUpdate, float LowTemperature,
                                                     float HighTemperature);

    /**
     * @brief Called to modulate the temps over the day
     *
     * @param Temperature
     * @param SecondsSinceUpdate
     * @param LowTemperature
     * @param HighTemperature
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Rainfall")
    float ModulateRainfall(float CurrentRainfallLevel, float SecondsSinceUpdate, float TargetRainfall);

    /**
     * @brief Native Implementation of ModulateRainfall
     *
     * @param Temperature
     * @param SecondsSinceUpdate
     * @param LowTemperature
     * @param HighTemperature
     * @return float
     */
    virtual float ModulateRainfall_Implementation(float CurrentRainfallLevel, float SecondsSinceUpdate,
                                                  float TargetRainfall);

    /**
     * @brief Modulate Fog by the current amount of precipitation
     *
     * @param FogHeight
     * @param SecondsSinceUpdate
     * @param RainLevel
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Functionality|Fog")
    float ModulateFogByRainfall(float FogHeight, float SecondsSinceUpdate, float RainLevel);

    /**
     * @brief Native Implementation of ModulateFogByRainfall
     *
     * @param FogHeight
     * @param SecondsSinceUpdate
     * @param RainLevel
     * @return float
     */
    virtual float ModulateFogByRainfall_Implementation(float FogHeight, float SecondsSinceUpdate, float RainLevel);

    /**
     * @brief Get the Rain Level
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Getters|Rainfall")
    float GetRainLevel();

    /**
     * @brief Native Implementation of GetRainLevel
     *
     * @return float
     */
    virtual float GetRainLevel_Implementation();

    /**
     * @brief Modulate Temperature by Location
     *
     * @param Temperature
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable, Category = "Climate|Functionality|Temperature")
    float ModulateTemperatureByLocation(float Temperature, FVector Location);

    /**
     * @brief Get the Updated Climate Data object
     *
     * @return FDateTimeClimateDataStruct
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Getters")
    FDateTimeClimateDataStruct GetUpdatedClimateData();

    /**
     * @brief Native Implementation of GetUpdatedClimateData
     *
     * @return FDateTimeClimateDataStruct
     */
    virtual FDateTimeClimateDataStruct GetUpdatedClimateData_Implementation();

    /**
     * @brief Get the Updated Climate Data object
     *
     * @return FDateTimeClimateDataStruct
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Climate|Getters")
    void GetClimateDataByRef(UPARAM(ref) FDateTimeClimateDataStruct &ClimateData);

    /**
     * @brief Native Implementation of GetUpdatedClimateData
     *
     * @return FDateTimeClimateDataStruct
     */
    virtual void GetClimateDataByRef_Implementation(UPARAM(ref) FDateTimeClimateDataStruct &ClimateData);
};
