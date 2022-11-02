// [TEMPLATE_COPYRIGHT]

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DateTimeSystemDataRows.h"
#include "DateTimeSystemComponent.h"

#include "ClimateComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("ClimateSystem"), STATGROUP_ACIClimateSys, STATCAT_Advanced);



UENUM(BlueprintType)
enum class FDateTimeClimateTypes : uint8
{
    Clear,
    Overcast,
    LightRain,
    HeavyRain,
    Monsoon,
    Fog,
    Snow,
    Hail,
    Frost,

    TOTAL_CLIMATE_TYPES
};

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
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangeDelegate, float, NewTemperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLocalDateTimeEvent);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UClimateComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* ClimateTable;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* ClimateOverridesTable;

    // Create ClimateBook and DateOverrides + Array to force lifetimes
    TArray<FDateTimeSystemClimateOverrideRow*> DOTemps;
    TMap< uint32, FDateTimeSystemClimateOverrideRow*> DateOverrides;
    TArray<FDateTimeSystemClimateMonthlyRow*> ClimateBook;

    UPROPERTY()
    UDateTimeSystemComponent *DateTimeSystem;

    // Stored
    float LastHighTemp;
    float LastLowTemp;

    bool SunHasRisen;

    // Cached Values
    FDateTimeSystemPackedCacheFloat CachedHighTemp;
    FDateTimeSystemPackedCacheFloat CachedLowTemp;
    FDateTimeSystemPackedCacheFloat CachedNextLowTemp;

    FDateTimeSystemPackedCacheFloat CachedPriorRH;
    FDateTimeSystemPackedCacheFloat CachedNextRH;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalRH;

    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyHighTemp;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyLowTemp;

    FDateTimeSystemStruct LocalTime;

public:
    UPROPERTY(BlueprintAssignable)
    FTemperatureChangeDelegate TemperatureChangeCallback;

    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunriseCallback;

    UPROPERTY(BlueprintAssignable)
    FLocalDateTimeEvent SunsetCallback;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float CurrentTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TemperatureChangeSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float CurrentRelativeHumidity;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float CurrentDewPoint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float SeaLevel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float ReferenceLatitude;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float ReferenceLongitude;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FDateTimeSystemTimezoneStruct TimezoneInfo;

private:
    void ClimateSetup();
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

private:
    float GetAnalyticalHighForDate(FDateTimeSystemStruct& DateStruct);
    float GetAnalyticalLowForDate(FDateTimeSystemStruct& DateStruct);

    float GetAnalyticalRHForDate(FDateTimeSystemStruct& DateStruct);

    float GetDailyHigh(FDateTimeSystemStruct& DateStruct);
    float GetDailyLow(FDateTimeSystemStruct& DateStruct);
    float GetDailyRH(FDateTimeSystemStruct& DateStruct);

    void UpdateCurrentTemperature(float DeltaTime);
    void UpdateCurrentClimate(float DeltaTime);

    UFUNCTION()
    void InternalDateChanged(FDateTimeSystemStruct& DateStruct);

    UDateTimeSystemComponent* FindComponent();

public:
    UClimateComponent();
    UClimateComponent(UClimateComponent& Other);
    UClimateComponent(const FObjectInitializer& ObjectInitializer);

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void BeginPlay();

    /**
     * Warning! This function may be off by up to one frame if GetLocalTime
     * Is called before InternalTick is.
     */
    UFUNCTION(BlueprintCallable)
    FDateTimeSystemStruct GetLocalTime();

    UFUNCTION(BlueprintCallable)
    float GetMonthlyHighTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    float GetMonthlyLowTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void InternalBegin();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void DateChanged(UPARAM(ref) FDateTimeSystemStruct& DateStruct);
    virtual void DateChanged_Implementation(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyLowModulation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyLowModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyHighModulation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyHighModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);

    // Called to modulate the temps over the day
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float ModulateTemperature(float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature);
    virtual float ModulateTemperature_Implementation(float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature);

    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperature();

    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperatureForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperature();

    UFUNCTION(BlueprintCallable)
    float GetCurrentFeltTemperatureForLocation(FVector Location);

    UFUNCTION(BlueprintCallable)
    float GetCloudLevel();

    UFUNCTION(BlueprintCallable)
    float GetHeatIndex();

    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVector(FVector WindVector);

    UFUNCTION(BlueprintCallable)
    float GetWindChillFromVelocity(float WindVelocity);

    UFUNCTION(BlueprintCallable)
    float GetWindChill();
};
