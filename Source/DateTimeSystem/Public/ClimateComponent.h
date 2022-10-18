// [TEMPLATE_COPYRIGHT]

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DateTimeSystemDataRows.h"
#include "DateTimeSystemComponent.h"

#include "ClimateComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("ClimateSystem"), STATGROUP_ACIClimateSys, STATCAT_Advanced);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangeDelegate, float, NewTemperature);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UClimateComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* YearBookTable;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* DateOverridesTable;

    // Create YearBook and DateOverrides + Array to force lifetimes
    TArray<FDateTimeSystemDateOverrideRow*> DOTemps;
    TMap< uint32, FDateTimeSystemDateOverrideRow*> DateOverrides;
    TArray<FDateTimeSystemYearbookRow*> YearBook;

    UPROPERTY()
    UDateTimeSystemComponent *DummyParentComponent;

    // Stored
    float LastHighTemp;
    float LastLowTemp;

    // Cached Values
    FDateTimeSystemPackedCacheFloat CachedHighTemp;
    FDateTimeSystemPackedCacheFloat CachedLowTemp;
    FDateTimeSystemPackedCacheFloat CachedNextLowTemp;

    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyHighTemp;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyLowTemp;

    FDateTimeSystemStruct LocalTime;

public:
    UPROPERTY(BlueprintAssignable)
    FTemperatureChangeDelegate TemperatureChangeCallback;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float CurrentTemperature;

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

    float GetDailyHigh(FDateTimeSystemStruct& DateStruct);
    float GetDailyLow(FDateTimeSystemStruct& DateStruct);

    void UpdateCurrentTemperature(float DeltaTime);

    void InternalDateChanged(FDateTimeSystemStruct& DateStruct);

public:
    UClimateComponent();
    UClimateComponent(UClimateComponent& Other);
    UClimateComponent(const FObjectInitializer& ObjectInitializer);

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
};
