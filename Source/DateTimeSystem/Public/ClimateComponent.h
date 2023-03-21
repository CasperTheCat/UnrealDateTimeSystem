// [TEMPLATE_COPYRIGHT]

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "DateTimeSystemComponent.h"
#include "DateTimeSystemDataRows.h"
#include "GameplayTagContainer.h"

#include "ClimateComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("ClimateSystem"), STATGROUP_ACIClimateSys, STATCAT_Advanced);

USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeClimateDataStruct
{
    GENERATED_BODY()

public:
    // Encodes how much water there is in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Wetness;

    // Amount of Rain to apply to shader
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Rain;

    // Accumulated Puddles
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Puddles;

    // Amount of frost
    // Similar to Wetness, but when cold
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Frost;

    // Snow. I use this for light snow
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Snow;

    // WindVector
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Wind;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChillOffset;

    // Temperataure
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeatOffset;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangeDelegate, float, NewTemperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateClimateData, FDateTimeClimateDataStruct, ClimateData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDTSUpdateTime, FDateTimeSystemStruct, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLocalDateTimeEvent);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UClimateComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    /**
     * @brief Tick rate when engine is ticking the component
     *
     */
    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    /**
     * @brief Climate Data Table
     * Uses FDateTimeSystemClimateMonthlyRow
     *
     */
    UPROPERTY(EditDefaultsOnly)
    UDataTable *ClimateTable;

    /**
     * @brief Climate Override Table
     * Uses FDateTimeSystemClimateOverrideRow
     *
     */
    UPROPERTY(EditDefaultsOnly)
    UDataTable *ClimateOverridesTable;

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
    UDateTimeSystemComponent *DateTimeSystem;

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
     * @brief Local Time Post Update
     *
     */
    UPROPERTY(Transient)
    FDateTimeSystemStruct LocalTime;

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
    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentTemperature;

    /**
     * @brief Computed Humidity
     *
     */
    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentRelativeHumidity;

    /**
     * @brief Computed Dew Point
     */
    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentDewPoint;

    /**
     * @brief Computed Fog Level
     *
     */
    UPROPERTY(SaveGame)
    float CurrentFog;

    /**
     * @brief Temperature Update Rate
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float TemperatureChangeSpeed;

    /**
     * @brief Fog Update Rate
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float FogChangeSpeed;

    /**
     * @brief Mean Sea Level
     * Used for offsetting the floor
     */
    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
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
     * @brief At what temperature do we skip to the new temperature due to us falling behind
     * Useful for non-contiguous time where blending would cause strange discontinuities
     *
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float TemperatureCatchupThreshold;

    /**
     * @brief At what angle below the horizon, in radians, does sunset callback trigger
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float SunPositionBelowHorizonThreshold;

    /**
     * @brief At what angle above the horizon, in radians, does sunrise callback trigger
     *
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float SunPositionAboveHorizonThreshold;

    /**
     * @brief Base Latitude relative to the DTS's reference
     *
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReferenceLatitude;

    /**
     * @brief Base Longitude relative to the DTS's reference
     *
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReferenceLongitude;

    /**
     * @brief Default Climate Update Rate
     * Use SetClimateUpdateFrequency to set in gameplay
     *
     */
    UPROPERTY(EditDefaultsOnly)
    float DefaultClimateUpdateFrequency;

    /**
     * @brief Timezone for this Climate Region
     * Does not support DST as that's hard
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    FDateTimeSystemTimezoneStruct TimezoneInfo;

    /**
     * @brief Direction of North Vector
     * Used to rotate Vectors
     * Defaults to +X North
     */
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
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
    void UpdateCurrentTemperature(float DeltaTime);

    /**
     * @brief Update the Current Climate
     *
     * @param DeltaTime
     */
    void UpdateCurrentClimate(float DeltaTime);

    /**
     * @brief Called by DTS via callback
     *
     */
    UFUNCTION()
    void InternalDateChanged(FDateTimeSystemStruct DateStruct);

    /**
     * @brief Find DTS
     *
     */
    UDateTimeSystemComponent *FindComponent();

    /**
     * @brief Internal function for adjusting Location Northing Vector
     *
     */
    FVector GetLocationAdjustedForNorthing(FVector Location);

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
    UFUNCTION(BlueprintCallable)
    FDateTimeSystemStruct GetLocalTime();

    /**
     * @brief Bind to the DTS
     * Normally called by InternalBegin
     *
     */
    UFUNCTION(BlueprintCallable)
    void BindToDateTimeSystem();

    /**
     * @brief Return whether the sun has risen
     *
     */
    UFUNCTION(BlueprintCallable)
    bool GetHasSunRisen()
    {
        return SunHasRisen;
    }

    /**
     * @brief Return whether the sun has set
     *
     */
    UFUNCTION(BlueprintCallable)
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
    UFUNCTION(BlueprintCallable)
    float GetMonthlyHighTemperature(int MonthIndex);

    /**
     * @brief Get the Monthly Low Temperature
     *
     * @param MonthIndex
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetMonthlyLowTemperature(int MonthIndex);

    /**
     * @brief Get the Current Temperature
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperature();

    /**
     * @brief Get the Current Temperature For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperatureForLocation(FVector Location);

    /**
     * @brief Get the Current Felt Temperature
     *
     * @param WindVelocity
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperature(float WindVelocity);

    /**
     * @brief Get the Current Felt Temperature For Location
     *
     * @param WindVelocity
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperatureForLocation(float WindVelocity, FVector Location);

    /**
     * @brief Get the Current Dew Point For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCurrentDewPointForLocation(FVector Location);

    /**
     * @brief Get the Relative Humidity For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetRelativeHumidityForLocation(FVector Location);

    /**
     * @brief Get the Cloud Level
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetCloudLevel();

    /**
     * @brief Get the Fog Level
     *
     * @param DeltaTime
     * @param Scale
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetFogLevel(float DeltaTime, float Scale = 1.f);

    /**
     * @brief Get the Heat Index
     *
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetHeatIndex();

    /**
     * @brief Get the Heat Index For Location
     *
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetHeatIndexForLocation(FVector Location);

    /**
     * @brief Get the Wind Chill From Vector
     *
     * @param WindVector
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVector(FVector WindVector);

    /**
     * @brief Get the Wind Chill From Velocity
     *
     * @param WindVelocity
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVelocity(float WindVelocity);

    /**
     * @brief Internal Tick
     * Call this is the engine isn't ticking the component
     * For example, when it is on a GameInstance
     *
     * @param DeltaTime
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
     * @brief Set the Climate Update Frequency
     *
     * @param Frequency
     */
    UFUNCTION(BlueprintCallable)
    void SetClimateUpdateFrequency(float Frequency);

    /**
     * @brief Get the Local Sun Rotation object
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    FRotator GetLocalSunRotation(FVector Location);

    /**
     * @brief Get the Local Moon Rotation object
     *
     * @param Location
     * @return FRotator
     */
    UFUNCTION(BlueprintCallable)
    FRotator GetLocalMoonRotation(FVector Location);

    /**
     * @brief Called internally when the date changed
     * No default implementation, override this to add features
     *
     * @param DateStruct
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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
     * @brief Modulate Fog by the current amount of precipitation
     *
     * @param FogHeight
     * @param SecondsSinceUpdate
     * @param RainLevel
     * @return float
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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
     * @brief Modulate Temperature by Location
     *
     * @param Temperature
     * @param Location
     * @return float
     */
    UFUNCTION(BlueprintCallable)
    float ModulateTemperatureByLocation(float Temperature, FVector Location);

    /**
     * @brief Get the Updated Climate Data object
     *
     * @return FDateTimeClimateDataStruct
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FDateTimeClimateDataStruct GetUpdatedClimateData();

    /**
     * @brief Native Implementation of GetUpdatedClimateData
     *
     * @return FDateTimeClimateDataStruct
     */
    virtual FDateTimeClimateDataStruct GetUpdatedClimateData_Implementation();
};
