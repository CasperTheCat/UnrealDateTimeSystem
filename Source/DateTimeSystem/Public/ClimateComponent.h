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
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateClimateData, UPARAM(ref) FDateTimeClimateDataStruct&,
// ClimateData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLocalDateTimeEvent);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UClimateComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    UPROPERTY(EditDefaultsOnly)
    UDataTable *ClimateTable;

    UPROPERTY(EditDefaultsOnly)
    UDataTable *ClimateOverridesTable;

    // Create ClimateBook and DateOverrides + Array to force lifetimes
    TArray<FDateTimeSystemClimateOverrideRow *> DOTemps;
    TMap<uint32, FDateTimeSystemClimateOverrideRow *> DateOverrides;
    TArray<FDateTimeSystemClimateMonthlyRow *> ClimateBook;

    UPROPERTY()
    UDateTimeSystemComponent *DateTimeSystem;

    // Stored
    float LastHighTemp;
    float LastLowTemp;

    float PercentileLatitude;
    float PercentileLongitude;
    float RadLatitude;
    float RadLongitude;
    float OneOverUpdateFrequency;
    float AccumulatedDeltaForCallback;

    bool SunHasRisen;
    bool SunHasSet;

    // Cached Values
    FDateTimeSystemPackedCacheFloat CachedHighTemp;
    FDateTimeSystemPackedCacheFloat CachedLowTemp;
    FDateTimeSystemPackedCacheFloat CachedNextLowTemp;

    FDateTimeSystemPackedCacheFloat CachedPriorDewPoint;
    FDateTimeSystemPackedCacheFloat CachedNextDewPoint;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalDewPoint;

    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyHighTemp;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyLowTemp;

    FDateTimeSystemStruct LocalTime;

public:
    UPROPERTY(BlueprintAssignable)
    FTemperatureChangeDelegate TemperatureChangeCallback;

    // Try to avoid this if you can
    UPROPERTY(BlueprintAssignable)
    FUpdateClimateData UpdateLocalClimateCallback;

    UPROPERTY(BlueprintAssignable)
    FDTSUpdateTime UpdateLocalTime;

    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunriseCallback;

    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunsetCallback;

    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent TwilightCallback;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentTemperature;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float TemperatureChangeSpeed;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    float FogChangeSpeed;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentRelativeHumidity;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float CurrentDewPoint;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float SeaLevel;

    UPROPERTY(SaveGame)
    bool HasBoundToDate;

    UPROPERTY(SaveGame)
    float CurrentFog;

    UPROPERTY()
    float DTSTimeScale;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float TemperatureCatchupThreshold;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float SunPositionBelowHorizonThreshold;

    UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
    float SunPositionAboveHorizonThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReferenceLatitude;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReferenceLongitude;

    UPROPERTY(EditDefaultsOnly)
    float DefaultClimateUpdateFrequency;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FDateTimeSystemTimezoneStruct TimezoneInfo;

private:
    void ClimateSetup();
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

    // Passthrough Function for LocalTime update
    UFUNCTION()
    void UpdateLocalTimePassthrough(FDateTimeSystemStruct NewTime);

private:
    float GetAnalyticalHighForDate(FDateTimeSystemStruct &DateStruct);
    float GetAnalyticalLowForDate(FDateTimeSystemStruct &DateStruct);

    float GetAnalyticalDewPointForDate(FDateTimeSystemStruct &DateStruct);

    float GetDailyHigh(FDateTimeSystemStruct &DateStruct);
    float GetDailyLow(FDateTimeSystemStruct &DateStruct);
    float GetDailyDewPoint(FDateTimeSystemStruct &DateStruct);

    void UpdateCurrentTemperature(float DeltaTime);
    void UpdateCurrentClimate(float DeltaTime);

    UFUNCTION()
    void InternalDateChanged(FDateTimeSystemStruct DateStruct);

    UDateTimeSystemComponent *FindComponent();

public:
    UClimateComponent();
    UClimateComponent(UClimateComponent &Other);
    UClimateComponent(const FObjectInitializer &ObjectInitializer);

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

    virtual void BeginPlay();

    /**
     * Warning! This function may be off by up to one frame if GetLocalTime
     * Is called before InternalTick is.
     */
    UFUNCTION(BlueprintCallable)
    FDateTimeSystemStruct GetLocalTime();

    UFUNCTION(BlueprintCallable)
    void BindToDateTimeSystem();

    UFUNCTION(BlueprintCallable)
    float GetMonthlyHighTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    float GetMonthlyLowTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void InternalBegin();

    UFUNCTION(BlueprintCallable)
    void SetClimateUpdateFrequency(float Frequency);

    UFUNCTION(BlueprintCallable)
    FRotator GetLocalSunRotation(FVector Location);

    UFUNCTION(BlueprintCallable)
    FRotator GetLocalMoonRotation(FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DateChanged(UPARAM(ref) FDateTimeSystemStruct &DateStruct);
    virtual void DateChanged_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyLowModulation(UPARAM(ref) FDateTimeSystemStruct &DateStruct, FGameplayTagContainer &Attributes,
                             float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyLowModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                    FGameplayTagContainer &Attributes, float Temperature,
                                                    float PreviousDayLow, float PreviousDayHigh);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyHighModulation(UPARAM(ref) FDateTimeSystemStruct &DateStruct, FGameplayTagContainer &Attributes,
                              float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyHighModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                     FGameplayTagContainer &Attributes, float Temperature,
                                                     float PreviousDayLow, float PreviousDayHigh);
    // Called to modulate the temps over the day
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float ModulateTemperature(float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature);
    virtual float ModulateTemperature_Implementation(float Temperature, float SecondsSinceUpdate, float LowTemperature,
                                                     float HighTemperature);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float ModulateFogByRainfall(float FogHeight, float SecondsSinceUpdate, float RainLevel);
    virtual float ModulateFogByRainfall_Implementation(float FogHeight, float SecondsSinceUpdate, float RainLevel);

    UFUNCTION(BlueprintCallable)
    float ModulateTemperatureByLocation(float Temperature, FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FDateTimeClimateDataStruct GetUpdatedClimateData();
    virtual FDateTimeClimateDataStruct GetUpdatedClimateData_Implementation();

    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperature();

    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperatureForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperature(float WindVelocity);

    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperatureForLocation(float WindVelocity, FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetCurrentDewPointForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetRelativeHumidityForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetCloudLevel();

    UFUNCTION(BlueprintCallable)
    float GetFogLevel(float DeltaTime, float Scale = 1.f);

    UFUNCTION(BlueprintCallable)
    float GetHeatIndex();

    UFUNCTION(BlueprintCallable)
    float GetHeatIndexForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVector(FVector WindVector);

    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVelocity(float WindVelocity);
};
