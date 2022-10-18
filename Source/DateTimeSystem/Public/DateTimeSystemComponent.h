// [TEMPLATE_COPYRIGHT]

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DateTimeSystemDataRows.h"

#include "DateTimeSystemComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("DateTimeSystem"), STATGROUP_ACIDateTimeSys, STATCAT_Advanced);

// Forward Decl
class UClimateComponent;

enum class EDateTimeSystemInvalidationTypes : uint8
{
    Frame,
    Day,
    Month,
    Year,

    TOTAL_INVALIDATION_TYPES
};

struct FDateTimeSystemPackedCacheFloat
{
    bool Valid;
    float Value;
};

struct FDateTimeSystemPackedCacheVector
{
    bool Valid;
    FVector Value;
};

struct FDateTimeSystemPackedCacheInt
{
    int32 Valid : 1;
    int32 Value : 31;
};


USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Seconds;

    UPROPERTY(BlueprintReadWrite)
    int Day;

    UPROPERTY(BlueprintReadWrite)
    int Month;

    UPROPERTY(BlueprintReadWrite)
    int Year;

    UPROPERTY(BlueprintReadWrite)
    int DayIndex;

public:
    float GetTimeInMinutes()
    {
        return Seconds / 60;
    }

    void SetFromRow(FDateTimeSystemDateOverrideRow* Row)
    {
        Seconds = 0;
        Day = Row->Day;
        Month = Row->Month;
        Year = Row->Year;
    }

    static FDateTimeSystemStruct CreateFromRow(FDateTimeSystemDateOverrideRow* Row)
    {
        auto RetVal = FDateTimeSystemStruct{};
        RetVal.Seconds = 0;
        RetVal.Day = Row->Day;
        RetVal.Month = Row->Month;
        RetVal.Year = Row->Year;
        RetVal.DayIndex = Row->DayIndex;

        return RetVal;
    }
};

FORCEINLINE uint32 GetTypeHash(const FDateTimeSystemStruct& Row)
{
    return Row.DayIndex;
    auto DIHash = GetTypeHash(Row.DayIndex);
    auto DHash = GetTypeHash(Row.Day);
    auto MHash = GetTypeHash(Row.Month);
    auto YHash = GetTypeHash(Row.Year);

    auto DateHash = HashCombine(HashCombine(DIHash, DHash), HashCombine(MHash, YHash));

    return DateHash;
}

USTRUCT(BlueprintType, Blueprintable)
struct FDateTimeSystemTimezoneStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float HoursDeltaFromMeridian;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDateChangeDelegate, FDateTimeSystemStruct&, NewDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOverridesDelegate, FDateTimeSystemStruct, NewDate, FGameplayTagContainer, Attribute);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATETIMESYSTEM_API UDateTimeSystemComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly)
    float LengthOfDay;

    float InvLengthOfDay;

    UPROPERTY(EditDefaultsOnly)
    float TicksPerSecond;

    UPROPERTY(SaveGame, EditAnywhere)
    float TimeScale;

    UPROPERTY(EditDefaultsOnly)
    float DaysInOrbitalYear;

    UPROPERTY(EditDefaultsOnly)
    float ReferenceLatitude;

    UPROPERTY(EditDefaultsOnly)
    float ReferenceLongitude;

    UPROPERTY(EditDefaultsOnly)
    float SolsticeOffsetInDays;

    UPROPERTY(EditDefaultsOnly)
    float PlanetRadius;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* YearBookTable;

    UPROPERTY(EditDefaultsOnly)
    UDataTable* DateOverridesTable;

    // UTC Time. To avoid conversions
    UPROPERTY(SaveGame, EditDefaultsOnly)
    FDateTimeSystemStruct InternalDate;

    UPROPERTY(EditDefaultsOnly)
    bool UseDayIndexForOverride;

    //UPROPERTY(SaveGame)
    //uint32 Day;

    //UPROPERTY(SaveGame)
    //uint32 Month;

    //UPROPERTY(SaveGame)
    //uint32 Year;

    //UPROPERTY(SaveGame)
    //uint32 DayIndex;

    // Create YearBook and DateOverrides + Array to force lifetimes
    TArray<FDateTimeSystemDateOverrideRow*> DOTemps;
    TMap< uint32, FDateTimeSystemDateOverrideRow*> DateOverrides;
    TArray<FDateTimeSystemYearbookRow*> YearBook;

    // Set when Yearbook is populated
    int LengthOfCalendarYearInDays;

    // Stored
    float LastHighTemp;
    float LastLowTemp;

    // Cached Values
    FDateTimeSystemPackedCacheFloat CachedHighTemp;
    FDateTimeSystemPackedCacheFloat CachedLowTemp;
    FDateTimeSystemPackedCacheFloat CachedNextLowTemp;

    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyHighTemp;
    FDateTimeSystemPackedCacheFloat CachedAnalyticalMonthlyLowTemp;

    FDateTimeSystemPackedCacheFloat CachedSolarFractionalYear;
    FDateTimeSystemPackedCacheFloat CachedSolarDeclinationAngle;
    FDateTimeSystemPackedCacheFloat CachedSolarTimeCorrection;

    FDateTimeSystemPackedCacheInt CachedDoesLeap;

    TMap<uint32, FVector> CachedSunVectors;

public:
    // Do we *really* want to save this and not just bake it with EditDefaults?
    UPROPERTY(SaveGame, EditAnywhere)
    bool OverridedDatesSetDate;

    UPROPERTY(BlueprintAssignable)
    FDateChangeDelegate DateChangeCallback;

    UPROPERTY(BlueprintAssignable)
    FOverridesDelegate DateOverrideCallback;

private:
    void DateTimeSetup();
    void Invalidate(EDateTimeSystemInvalidationTypes Type);

private:
    uint32 GetHashForDate(FDateTimeSystemStruct *DateStruct);
    uint32 GetHashForDate(FDateTimeSystemDateOverrideRow* DateStruct);
    FDateTimeSystemDateOverrideRow** GetDateOverride(FDateTimeSystemStruct* DateStruct);

    bool HandleDayRollover(FDateTimeSystemStruct& DateStruct);
    bool HandleMonthRollover(FDateTimeSystemStruct& DateStruct);
    bool HandleYearRollover(FDateTimeSystemStruct& DateStruct);

    float GetAnalyticalHighForDate(FDateTimeSystemStruct& DateStruct);
    float GetAnalyticalLowForDate(FDateTimeSystemStruct& DateStruct);

    float GetDailyHigh(FDateTimeSystemStruct& DateStruct);
    float GetDailyLow(FDateTimeSystemStruct& DateStruct);

    int GetLengthOfCalendarYear(int Year);


    float GetDeclinationAngle(FDateTimeSystemStruct& DateStruct);

    int GetDaysInCurrentMonth();
    int GetDaysInMonth(int MonthIndex);
    int GetMonthsInYear(int YearIndex);

    bool SanitiseDateTime(FDateTimeSystemStruct& DateStruct);

    float GetSolarFractionalDay();
    float GetSolarFractionalYear();
    float SolarTimeCorrection(float YearInRadians);
    float SolarDeclinationAngle(float YearInRadians);

    bool InternalDoesLeap(int Year);


public:
    UDateTimeSystemComponent();
    UDateTimeSystemComponent(UDateTimeSystemComponent& Other);
    UDateTimeSystemComponent(const FObjectInitializer& ObjectInitializer);
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void BeginPlay();

    UFUNCTION(BlueprintCallable)
    void GetTodaysDate(FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    void GetTodaysDateTZ(FDateTimeSystemStruct& DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);

    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDate(FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    void GetTomorrowsDateTZ(FDateTimeSystemStruct& DateStruct, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLatitudeFromLocation(FVector Location);
    virtual float GetLatitudeFromLocation_Implementation(FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float GetLongitudeFromLocation(FVector Location);
    virtual float GetLongitudeFromLocation_Implementation(FVector Location);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    bool DoesYearLeap(int Year);
    virtual bool DoesYearLeap_Implementation(int Year);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotationForLocation(FVector Location, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);
    virtual FRotator GetSunRotationForLocation_Implementation(FVector Location, FDateTimeSystemTimezoneStruct& TimezoneInfo);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRotator GetSunRotation(FDateTimeSystemTimezoneStruct TimezoneInfo);
    virtual FRotator GetSunRotation_Implementation(FDateTimeSystemTimezoneStruct TimezoneInfo);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FVector GetSunVector(float Latitude, float Longitude);
    virtual FVector GetSunVector_Implementation(float Latitude, float Longitude);

    UFUNCTION(BlueprintCallable)
    float GetFractionalDay(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalMonth(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalOrbitalYear(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetFractionalCalendarYear(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable)
    float GetMonthlyHighTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    float GetMonthlyLowTemperature(int MonthIndex);

    UFUNCTION(BlueprintCallable)
    void DummyAddTick(float Time);

    UFUNCTION(BlueprintCallable)
    void InternalTick(float DeltaTime);

    UFUNCTION(BlueprintCallable)
    void InternalBegin();

    UFUNCTION(BlueprintCallable)
    FName GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct& DateStruct);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyLowModulation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyLowModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float DailyHighModulation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);
    virtual float DailyHighModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh);


    // Called to modulate the temps over the day
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    float ModulateTemperature(FVector Location, float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);
    virtual float ModulateTemperature_Implementation(FVector Location, float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);

    UFUNCTION(BlueprintCallable)
    float GetCurrentTemperature(FVector Location, float CurrentTemperature, float SecondsSinceUpdate, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo);

    //UFUNCTION(BlueprintCallable)
    //static float GetSeconds
    // Crazy Function to set local location

    friend class UClimateComponent;

};
