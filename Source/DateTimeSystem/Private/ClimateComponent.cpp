// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "ClimateComponent.h"
#include "DateTimeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameState.h"
#include "DateTimeCommonCore.h"

void UClimateComponent::ClimateSetup()
{
    IsInitialised = false;
    TicksPerSecond = 0;
    RainProbabilityMultiplier = 1.f;
    CatchupThresholdInSeconds = 30;
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    RegisterAllComponentTickFunctions(true);
    HasBoundToDate = false;
    DefaultClimateUpdateFrequency = 30;
    TemperatureChangeSpeed = 0.001;
    FogChangeSpeed = 0.001;
    DTSTimeScale = 1.f;
    SunHasRisen = false;
    SunHasSet = false;
    NorthingDirection = FVector::ForwardVector;

    SunPositionBelowHorizonThreshold = 0.087155f;
    SunPositionAboveHorizonThreshold = 0.0435775f;

    NumberOfRainSlotsPerDay = 24;
    UseSunPositionForEvaporation = false;
    CatchupWetnessDownblendSpeed = 5;
    CatchupWetnessUpblendSpeed = 15;
    CatchupSittingWaterLimit = 0.5f;
    WetnessEvaporationRate = 35;
    WetnessDepositionRate = 0.2;
    WetnessEvaporationRateBase = 2;
    RainfallBlendIncreaseSpeed = 0.04;
    RainfallBlendDecreaseSpeed = 0.12;

    RainfallWetnessOverflowPuddlingScale = 0.02f;
    PuddleEvaporationRate = 12.5;
    PuddleEvaporationRateBase = 2;
    PuddleLimit = 6.5f;

    CurrentWetnessLimit = 1.f;
    CurrentSittingWaterLimit = 100.f;
}

void UClimateComponent::Invalidate(EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Invalidate"), STAT_ACICSInvalidate, STATGROUP_ACIClimateSys);

    if (Type >= EDateTimeSystemInvalidationTypes::Month)
    {
    }

    // Not cleared on frame-to-frame invalidation
    if (Type >= EDateTimeSystemInvalidationTypes::Day)
    {
        CachedHighTemp.Valid = false;
        CachedLowTemp.Valid = false;
        CachedNextLowTemp.Valid = false;
        CachedAnalyticalMonthlyHighTemp.Valid = false;
        CachedAnalyticalMonthlyLowTemp.Valid = false;
        CachedAnalyticalDewPoint.Valid = false;
        CachedNextDewPoint.Valid = false;
        CachedPriorDewPoint.Valid = false;
        CachedPriorRainfall.Valid = false;
        CachedNextRainfall.Valid = false;
    }

    CachedProbability.Empty();
    CachedAnalyticProbability.Empty();
    CachedRainfallLevels.Empty();
    CachedAnalyticRainfallLevel.Empty();

    if (InvalidationCallback.IsBound())
    {
        InvalidationCallback.Broadcast(Type);
    }
}

void UClimateComponent::UpdateLocalTimePassthrough()
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UpdateLocalTimePassthrough"), STAT_ACICSUpdateLocalTimePassthrough,
                                STATGROUP_ACIClimateSys);
    FDateTimeSystemStruct Local{};

    // Check again, for consistency
    if (DateTimeSystem)
    {
        // Update Local Time. We need it for a few things
        DateTimeSystem->GetTodaysDateTZ(Local, TimezoneInfo);
    }

    if (UpdateLocalTime.IsBound())
    {
        UpdateLocalTime.Broadcast(Local);
    }

    if (LocalTimeUpdateSignal.IsBound())
    {
        LocalTimeUpdateSignal.Broadcast();
    }
}

FDateTimeSystemStruct UClimateComponent::GetLocalTime()
{
    // SunHasRisen = false;
    return LocalTime;
}

void UClimateComponent::BindToDateTimeSystem()
{
    if (DateTimeSystem && DateTimeSystem->GetCore() && (UpdateLocalTime.IsBound() || LocalTimeUpdateSignal.IsBound()))
    {
        // If someone is listening to the update, we pass it through
        DateTimeSystem->GetCore()->CleanTimeUpdate.AddDynamic(this, &UClimateComponent::UpdateLocalTimePassthrough);
    }
}

float UClimateComponent::GetMonthlyHighTemperature(int MonthIndex)
{
    if (MonthIndex < ClimateBook.Num())
    {
        return ClimateBook[MonthIndex]->MonthlyHighTemp;
    }
    return 0.0f;
}

float UClimateComponent::GetMonthlyLowTemperature(int MonthIndex)
{
    if (MonthIndex < ClimateBook.Num())
    {
        return ClimateBook[MonthIndex]->MonthlyLowTemp;
    }
    return 0.0f;
}

float UClimateComponent::DailyLowModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                           FGameplayTagContainer &Attributes, float Temperature,
                                                           float PreviousDayLow, float PreviousDayHigh)
{
    return Temperature;
}

float UClimateComponent::DailyHighModulation_Implementation(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                            FGameplayTagContainer &Attributes, float Temperature,
                                                            float PreviousDayLow, float PreviousDayHigh)
{
    return Temperature;
}

float UClimateComponent::ModulateTemperature_Implementation(float Temperature, float SecondsSinceUpdate,
                                                            float LowTemperature, float HighTemperature)
{
    // Get the sun vector. We want to suppress the temperature when it's below the horizon
    if (DateTimeSystem)
    {

        auto SunVector = DateTimeSystem->GetSunVector(RadLatitude, RadLongitude);

        const auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
        const auto Multi = FMath::Sin(PI * FracDay);
        return FMath::Lerp(LowTemperature, HighTemperature, Multi);

        //// Using a proportional control
        // auto SunPower = FVector::DotProduct(SunVector, FVector::UpVector);

        // auto ClampedValues = FMath::Clamp(SunPower + 0.5, 0.f, 1.f);
        // auto SunTarget = FMath::Lerp(LowTemperature, HighTemperature, ClampedValues); // SunPower * 0.5 + 0.5);
        // return SunTarget;

        // auto SunError = (SunTarget - Temperature);

        // if (FMath::Abs(SunError) > TemperatureCatchupThreshold)
        //{
        //     //
        //     GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
        //                                      FString("Catching up to analytic.") + FString::SanitizeFloat(SunError));
        //     return SunTarget;
        // }

        // return Temperature + SunError * TemperatureChangeSpeed * SecondsSinceUpdate * DTSTimeScale;
    }

    return 0.f;
}

float UClimateComponent::ModulateRainfall_Implementation(float CurrentRainfallLevel, float SecondsSinceUpdate,
                                                         float TargetRainfall)
{
    if (FMath::IsNearlyEqual(CurrentRainfallLevel, TargetRainfall, KINDA_SMALL_NUMBER))
    {
        return TargetRainfall;
    }
    // We are increasing
    else if (TargetRainfall > CurrentRainfall)
    {
        return FMath::FInterpTo(CurrentRainfallLevel, TargetRainfall, SecondsSinceUpdate, RainfallBlendIncreaseSpeed);
    }
    else
    {
        return FMath::FInterpTo(CurrentRainfallLevel, TargetRainfall, SecondsSinceUpdate, RainfallBlendDecreaseSpeed);
    }
}

float UClimateComponent::ModulateFogByRainfall_Implementation(float FogHeight, float SecondsSinceUpdate,
                                                              float RainLevel)
{
    return FogHeight;
}

float UClimateComponent::GetRainLevel_Implementation()
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetRainLevel"), STAT_ACICSGetRainLevel, STATGROUP_ACIClimateSys);

    // We might want to cache?
    if (DateTimeSystem && NumberOfRainSlotsPerDay > 0)
    {
        // For Blending, are we blending forward, or backward?
        const auto FracBin = LocalTime.GetFractionalBin(DateTimeSystem->GetLengthOfDay(), NumberOfRainSlotsPerDay);
        auto Offset = DateTimeSystem->GetLengthOfDay() / NumberOfRainSlotsPerDay;

        if (FracBin < 0.5)
        {
            Offset *= -1;
        }

        // Copy the object
        FDateTimeSystemStruct OffsetTime = LocalTime;
        OffsetTime.Seconds += Offset;
        DateTimeSystem->SanitiseDateTime(OffsetTime);

        // Get Hash of the hour
        const double BinHash = LocalTime.GetBinHash(DateTimeSystem->GetLengthOfDay(), NumberOfRainSlotsPerDay);
        float Probability = BinHash / UINT32_MAX;

        const double OffsetHash = OffsetTime.GetBinHash(DateTimeSystem->GetLengthOfDay(), NumberOfRainSlotsPerDay);
        const float OffsetProb = OffsetHash / UINT32_MAX;

        Probability = FMath::Lerp(Probability, OffsetProb, FMath::Abs(FracBin - 0.5));

        // If Below, we want to rain down
        if (Probability * RainProbabilityMultiplier < GetPrecipitationThreshold(LocalTime))
        {
            CurrentPrecipitationLevel = GetRainfallAmount(LocalTime);
            // auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
            return CurrentPrecipitationLevel;
        }
    }
    return 0.0f;
}

float UClimateComponent::ModulateTemperatureByLocation(float Temperature, FVector Location)
{
    const auto AltitudeAboveSeaLevel = Location.Z - SeaLevel;
    return Temperature - (AltitudeAboveSeaLevel * 0.000065);
}

FDateTimeClimateDataStruct UClimateComponent::GetUpdatedClimateData_Implementation()
{
    const auto WindVector = FVector(0, 0, 0);

    FDateTimeClimateDataStruct Returnable{};
    Returnable.Temperature = GetCurrentTemperature();
    Returnable.HeatOffset = GetHeatIndex();
    Returnable.Wind = WindVector;
    Returnable.ChillOffset = GetWindChillFromVector(WindVector);
    Returnable.Rain = GetCurrentRainfall();
    Returnable.Wetness = GetCurrentWetness();
    Returnable.Puddles = GetCurrentSittingWater();

    return Returnable;
}

void UClimateComponent::GetClimateDataByRef_Implementation(FDateTimeClimateDataStruct &ClimateData)
{
    const auto WindVector = FVector(0, 0, 0);

    ClimateData.Temperature = GetCurrentTemperature();
    ClimateData.HeatOffset = GetHeatIndex();
    ClimateData.Wind = WindVector;
    ClimateData.ChillOffset = GetWindChillFromVector(WindVector);
    ClimateData.Rain = GetCurrentRainfall();
    ClimateData.Wetness = GetCurrentWetness();
    ClimateData.Puddles = GetCurrentSittingWater();
}

void UClimateComponent::SetClimateTable(TObjectPtr<UDataTable> NewClimateTable, bool ForceReinitialise)
{
    // Check init state
    if (IsInitialised && !ForceReinitialise)
    {
        UE_LOG(LogClimateSystem, Error, TEXT("SetClimateTable called on initialised table"));
    }
    ClimateTable = NewClimateTable;

    if (IsInitialised && ForceReinitialise)
    {
        UE_LOG(LogClimateSystem, Log, TEXT("SetClimateTable called on initialised table. Performing Reinit"));
        InternalBegin();
    }
}

void UClimateComponent::SetClimateOverridesTable(TObjectPtr<UDataTable> NewClimateOverrideTable, bool ForceReinitialise)
{
    // Check init state
    if (IsInitialised && !ForceReinitialise)
    {
        UE_LOG(LogClimateSystem, Error, TEXT("SetClimateTable called on initialised table"));
    }
    ClimateOverridesTable = NewClimateOverrideTable;

    if (IsInitialised && ForceReinitialise)
    {
        UE_LOG(LogClimateSystem, Log, TEXT("SetClimateTable called on initialised table. Performing Reinit"));
        InternalBegin();
    }
}

void UClimateComponent::UpdateCurrentTemperature(float DeltaTime, bool NonContiguous)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UpdateCurrentTemperature"), STAT_ACICSUpdateCurrentTemperature,
                                STATGROUP_ACIClimateSys);

    // Which low are we using? The first or last?
    if (DateTimeSystem)
    {
        const auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);

        const auto HighTemp = GetDailyHigh(LocalTime);
        auto LowTemp = CachedLowTemp.Value;
        if (FracDay > 0.5)
        {
            // Need today's low, rather than the prior
            LowTemp = GetDailyLow(LocalTime);
        }

        CurrentTemperature = ModulateTemperature(CurrentTemperature, DeltaTime, LowTemp, HighTemp);
    }
}

void UClimateComponent::UpdateCurrentRainfall(float DeltaTime, bool NonContiguous)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UpdateCurrentRainfall"), STAT_ACICSUpdateCurrentRainfall,
                                STATGROUP_ACIClimateSys);

    // Which low are we using? The first or last?
    if (DateTimeSystem)
    {
        const auto TargetRainfall = GetRainLevel();

        if (NonContiguous)
        {
            const auto FracBin = LocalTime.GetFractionalBin(DateTimeSystem->GetLengthOfDay(), NumberOfRainSlotsPerDay);
            const auto Offset = DateTimeSystem->GetLengthOfDay() / NumberOfRainSlotsPerDay;

            // If the last bin was rain, we need to care about this!
            FDateTimeSystemStruct PastTime = LocalTime;
            PastTime.Seconds += Offset;
            DateTimeSystem->SanitiseDateTime(PastTime);

            const double PastHash = PastTime.GetBinHash(DateTimeSystem->GetLengthOfDay(), NumberOfRainSlotsPerDay);
            const float PastProb = PastHash / UINT32_MAX;

            const auto DidRainLastBin = PastProb < GetPrecipitationThreshold(PastTime);

            // If it rained, but no longer is raining. We want to use downblend threshold
            if (DidRainLastBin && TargetRainfall < KINDA_SMALL_NUMBER)
            {
                const auto BlendLevel = FMath::Min(1.f, FracBin * CatchupWetnessDownblendSpeed);
                CurrentWetness = FMath::Lerp(1.f, 0.f, BlendLevel);
                CurrentSittingWater = FMath::Lerp(CatchupSittingWaterLimit, 0.f, BlendLevel * BlendLevel);
            }

            // Else, if it didn't rain, but now is, upblend
            else if (!DidRainLastBin && TargetRainfall > 0.f)
            {
                const auto BlendLevel = FMath::Min(1.f, FracBin * CatchupWetnessUpblendSpeed);
                CurrentWetness = FMath::Lerp(0.f, 1.f, BlendLevel);
                CurrentSittingWater = FMath::Lerp(0.f, CatchupSittingWaterLimit, BlendLevel * BlendLevel);
            }

            // Else, if did rain, and still is raining, wetness is 100%
            else if (DidRainLastBin && TargetRainfall > 0.f)
            {
                CurrentWetness = 1.f;
                CurrentSittingWater = CatchupSittingWaterLimit;
            }

            // Else, if it didn't rain, and still isn't, wetness is 0%
            else if (!DidRainLastBin && TargetRainfall < KINDA_SMALL_NUMBER)
            {
                CurrentWetness = 0.f;
                CurrentSittingWater = 0.f;
            }

            else
            {
                // How?
                checkNoEntry();
            }

            CurrentRainfall = TargetRainfall;
        }
        else
        {
            // Rainfall
            CurrentRainfall = ModulateRainfall(CurrentRainfall, DeltaTime, TargetRainfall);

            // Handle Live Wetness
            auto WetnessProxy = CurrentWetness;
            const auto DeltaTimeInMinutes = DeltaTime * 0.01666666666666666666666666666667f;

            // SunPositionBlend
            auto SunPositionBlend = 0.f;

            // Tied to SunPosition and reused
            // Measured in Percent Per Minute
            if (UseSunPositionForEvaporation)
            {
                // Cached in the event of SunRisen or SunSet being used
                const auto SunVector = DateTimeSystem->GetSunVector(RadLatitude, RadLongitude);
                const float VectorDot = FVector::DotProduct(SunVector, FVector::UpVector);

                SunPositionBlend = FMath::Max(0, VectorDot);
            }
            else
            {
                // Fall back to using blended fractional day
                const auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
                const auto InvertedBlend = FMath::Abs((FracDay * 2.f) - 1.f);
                const auto TighterBlend = FMath::Min(InvertedBlend * 1.66f, 1);

                SunPositionBlend = 1 - TighterBlend;
            }

            // Wetness
            {
                const auto EvaporationCoeff =
                    FMath::Lerp(WetnessEvaporationRateBase, WetnessEvaporationRate, SunPositionBlend);

                WetnessProxy -= EvaporationCoeff * 0.01f * DeltaTimeInMinutes * WetnessProxy;
                WetnessProxy += CurrentRainfall * WetnessDepositionRate * DeltaTimeInMinutes;

                if (WetnessProxy < KINDA_SMALL_NUMBER)
                {
                    WetnessProxy = 0.f;
                }
            }

            // Clamp Wetness
            CurrentWetness = FMath::Min(1.f, WetnessProxy);

            // Puddling Overflow
            // Evaporate
            {
                const auto EvaporationCoeff = FMath::Lerp(PuddleEvaporationRateBase, PuddleEvaporationRate,
                                                          SunPositionBlend);

                CurrentSittingWater -= EvaporationCoeff * 0.01f * DeltaTimeInMinutes * CurrentSittingWater;

                CurrentSittingWater += FMath::Max(0.f, WetnessProxy - 1.f) * RainfallWetnessOverflowPuddlingScale;

                CurrentSittingWater = FMath::Min(PuddleLimit, CurrentSittingWater);

                if (CurrentSittingWater < KINDA_SMALL_NUMBER)
                {
                    CurrentSittingWater = 0.f;
                }
            }
        }
    }
}

void UClimateComponent::UpdateCurrentClimate(float DeltaTime, bool NonContiguous)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UpdateCurrentClimate"), STAT_ACICSUpdateCurrentClimate, STATGROUP_ACIClimateSys);
    if (DateTimeSystem)
    {
        // Rel. Humidity
        const auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);

        //
        const auto NextRH = GetDailyDewPoint(LocalTime);
        const auto LowRH = CachedPriorDewPoint.Value;
        CurrentDewPoint = FMath::Lerp(LowRH, NextRH, FracDay);

        // Ru
        const auto LogRH = (CurrentDewPoint * 18.678f) / (257.14f + CurrentDewPoint) -
                           (CurrentTemperature * 18.678f) / (257.14f + CurrentTemperature);

        // RH
        CurrentRelativeHumidity = FMath::Exp(LogRH);
    }
}

void UClimateComponent::InternalDateChanged(FDateTimeSystemStruct DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("InternalDateChanged"), STAT_ACICSInternalDateChanged, STATGROUP_ACIClimateSys);
    // Handle what was once done on InternalTick
    Invalidate(EDateTimeSystemInvalidationTypes::Day);

    // Handle rolling the starting temp of day n+1 to the ending of n
    CachedLowTemp.Value = CachedNextLowTemp.Value;
    CachedLowTemp.Valid = true;
    LastHighTemp = CachedHighTemp.Value;
    LastLowTemp = CachedLowTemp.Value;
    CachedPriorDewPoint.Value = CachedNextDewPoint.Value;

    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            // Temperatures
            CachedNextLowTemp.Value = DailyLowModulation(DateStruct, asPtr->MiscData, asPtr->LowTemp,
                                                         CachedLowTemp.Value, CachedHighTemp.Value);
            CachedHighTemp.Value = DailyHighModulation(DateStruct, asPtr->MiscData, asPtr->HighTemp,
                                                       CachedLowTemp.Value, CachedHighTemp.Value);
            CachedNextLowTemp.Valid = true;
            CachedHighTemp.Valid = true;
        }
    }
    else
    {
        // Go ahead and compute the new values
        CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(DateStruct);
        CachedAnalyticalMonthlyHighTemp.Valid = true;
        CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(DateStruct);
        CachedAnalyticalMonthlyLowTemp.Valid = true;

        auto DummyTagContainer = FGameplayTagContainer();

        CachedNextLowTemp.Value =
            DailyLowModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, CachedLowTemp.Value,
                               CachedHighTemp.Value);
        CachedNextLowTemp.Valid = true;
        CachedHighTemp.Value = DailyHighModulation(DateStruct, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value,
                                                   CachedLowTemp.Value, CachedHighTemp.Value);
        CachedHighTemp.Valid = true;
    }

    // Date has changed
    // Call the BP function
    DateChanged(DateStruct);
}

TScriptInterface<IDateTimeSystemCommon> UClimateComponent::FindComponent()
{
    const auto World = GetWorld();
    if (World)
    {
        // If we're here, Instance failed to get the time system.
        const auto GameState = World->GetGameState();
        if (GameState)
        {
            // BP Casting Method
            if (GameState->GetClass()->ImplementsInterface(UDateTimeSystemInterface::StaticClass()))
            {
                auto Temp = IDateTimeSystemInterface::Execute_GetDateTimeSystem(GameState);

                if (IsValid(Temp) && Temp->GetClass()->ImplementsInterface(UDateTimeSystemCommon::StaticClass()))
                {
                    return Temp;
                }
            }
        }

        const auto GameInst = World->GetGameInstance();
        if (GameInst)
        {
            // BP Casting Method
            if (GameInst->GetClass()->ImplementsInterface(UDateTimeSystemInterface::StaticClass()))
            {
                auto Temp = IDateTimeSystemInterface::Execute_GetDateTimeSystem(GameInst);

                if (IsValid(Temp) && Temp->GetClass()->ImplementsInterface(UDateTimeSystemCommon::StaticClass()))
                {
                    return Temp;
                }
            }
            else
            {
                // Use Global
                auto GlobalInstance = GameInst->GetSubsystem<UDateTimeSystem>();
                return GlobalInstance;
            }
        }
    }

    return nullptr;
}

FORCEINLINE FVector UClimateComponent::GetLocationAdjustedForNorthing(FVector Location)
{
    if (DateTimeSystem)
    {
        return DateTimeSystem->AlignWorldLocationInternalCoordinates(Location, NorthingDirection);
    }

    return Location;
}

FVector UClimateComponent::RotateByNorthing(FVector Location)
{
    if (DateTimeSystem)
    {
        return DateTimeSystem->RotateLocationByNorthing(Location, NorthingDirection);
    }

    return Location;
}

FRotator UClimateComponent::RotateByNorthing(FRotator Rotation)
{
    if (DateTimeSystem)
    {
        return DateTimeSystem->RotateRotationByNorthing(Rotation, NorthingDirection);
    }

    return Rotation;
}

FMatrix UClimateComponent::RotateByNorthing(const FMatrix &Rotation)
{
    if (DateTimeSystem)
    {
        return DateTimeSystem->RotateMatrixByNorthing(Rotation, NorthingDirection);
    }

    return Rotation;
}

float UClimateComponent::GetCurrentTemperature()
{
    return CurrentTemperature;
}

float UClimateComponent::GetCurrentRainfall()
{
    return CurrentRainfall;
}

float UClimateComponent::GetCurrentWetness()
{
    return FMath::Min(CurrentWetnessLimit, CurrentWetness);
}

float UClimateComponent::GetCurrentSittingWater()
{
    return FMath::Min(CurrentSittingWaterLimit, CurrentSittingWater);
}

float UClimateComponent::DebugGetUnclampedWetness()
{
    return CurrentWetness + CurrentSittingWater;
}

float UClimateComponent::GetCurrentTemperatureForLocation(FVector Location)
{
    return ModulateTemperatureByLocation(CurrentTemperature, Location);
}

float UClimateComponent::GetCurrentFeltTemperature(float WindVelocity)
{
    return GetCurrentTemperature() + GetHeatIndex() - GetWindChillFromVelocity(WindVelocity);
}

float UClimateComponent::GetCurrentFeltTemperatureForLocation(float WindVelocity, FVector Location)
{
    return ModulateTemperatureByLocation(GetCurrentFeltTemperature(WindVelocity), Location);
}

float UClimateComponent::GetCurrentDewPointForLocation(FVector Location)
{
    // Lapse the dew point
    const auto AltitudeAboveSeaLevel = Location.Z - SeaLevel;
    return CurrentDewPoint - (AltitudeAboveSeaLevel * 0.000018);
}

float UClimateComponent::GetRelativeHumidityForLocation(FVector Location)
{
    const auto GetLocalDewPoint = GetCurrentDewPointForLocation(Location);

    const auto LogRH = (GetLocalDewPoint * 18.678f) / (257.14f + GetLocalDewPoint) -
                       (CurrentTemperature * 18.678f) / (257.14f + CurrentTemperature);

    // RH
    return FMath::Exp(LogRH);
}

float UClimateComponent::GetCloudLevel()
{
    // Previously used 6.5C as our lapse rate
    // This is incorrect
    // Dry adiabatic lapse is 9.8, and dew point lapse is around 1.8 (1.6-1.9)
    // This gives us 8 as our lapse
    const auto CloudHeightMetres = FMath::Abs(CurrentTemperature - CurrentDewPoint) * (1000 / 6.5f);

    return (CloudHeightMetres * 100);
}

float UClimateComponent::GetFogLevel(float DeltaTime, float Scale)
{
    // Fog is strange
    // Fog forms when Delta(Td, T) is less than 2.5C

    const auto Delta = FMath::Abs(CurrentTemperature - CurrentDewPoint);
    auto TargetFogLevel = 0.f;

    const auto TargetMultiplier = FMath::Clamp((Delta - 2.f) * 2, 0.f, 1.f);

    if (Delta < 2.5f)
    {
        // Okay, so we have fog. But what's the height?
        // Let's go with linear temperature's below 4C
        // where 1C is 1m
        // The issue is that obviously -35 doesn't translate
        // To 31m of fog. It's probably too cold for it

        TargetFogLevel = 4 - CurrentTemperature;
    }

    TargetFogLevel = (4 - CurrentTemperature) * (1 - TargetMultiplier);

    const auto TFL = (TargetFogLevel - CurrentFog) * FogChangeSpeed * DeltaTime * DTSTimeScale;
    CurrentFog = CurrentFog + TFL;

    const auto CL = FMath::Abs(TargetFogLevel) * Scale * 100 + SeaLevel;

    // checkNoEntry();
    // auto CL = -GetCloudLevel();
    return ModulateFogByRainfall(CL, DeltaTime, GetRainLevel());
}

float UClimateComponent::GetHeatIndex()
{
    return GetHeatIndexForLocation(FVector(0, 0, SeaLevel));
}

float UClimateComponent::GetHeatIndexForLocation(FVector Location)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetHeatIndexForLocation"), STAT_ACICSGetHeatIndexForLocation,
                                STATGROUP_ACIClimateSys);
    // auto RelativeMSL(Location.Z - SeaLevel);
    const auto T = GetCurrentTemperatureForLocation(Location);
    //    auto R = CurrentRelativeHumidity;
    const auto R = GetRelativeHumidityForLocation(Location);
    const auto TT = T * T;
    const auto RR = R * R;

    // Constants
    const auto C1 = -8.78469475556f;
    const auto C2 = 1.61139411f;
    const auto C3 = 2.33854883889f;
    const auto C4 = -0.14611605f;
    const auto C5 = -0.012308084f;
    const auto C6 = -0.0164248277778f;
    const auto C7 = 2.211732e-3f;
    const auto C8 = 7.2546e-4f;
    const auto C9 = -3.582e-6f;

    const auto HeatIndex = C1
                           + C2 * T
                           + C3 * R
                           + C4 * T * R
                           + C5 * TT
                           + C6 * RR
                           + C7 * TT * R
                           + C8 * T * RR
                           + C9 * TT * RR;

    //// We want to mute HI when temp is under 25C
    //// We can do this rolling off
    const auto HeatIndexDelta = HeatIndex - CurrentTemperature;

    return FMath::Lerp(0, HeatIndexDelta, FMath::Clamp(CurrentTemperature - 25, 0, 1));
}

float UClimateComponent::GetWindChillFromVector(FVector WindVector)
{
    return GetWindChillFromVelocity(WindVector.Length());
}

float UClimateComponent::GetWindChillFromVelocity(float WindVelocity)
{
    const auto T = CurrentTemperature;
    const auto V = FMath::Pow(WindVelocity, 0.16);

    const auto WC = 13.12 + 0.6215 * T - 11.37 * V + 0.3965 * T * V;

    return FMath::Min(0, WC - CurrentTemperature);
}

void UClimateComponent::InternalTick(float DeltaTime)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("InternalTick"), STAT_ACICSInternalTick, STATGROUP_ACIClimateSys);

    Invalidate(EDateTimeSystemInvalidationTypes::Frame);

    if (DateTimeSystem && DateTimeSystem->IsReady())
    {
        // Update Local Time. We need it for a few things
        PriorLocalTime = LocalTime;
        DateTimeSystem->GetTodaysDateTZ(LocalTime, TimezoneInfo);

        // Check for the delta
        // auto Delta = FMath::Abs(LocalTime.Seconds - PriorLocalTime.Seconds);
        const auto Delta = DateTimeSystem->ComputeDeltaBetweenDatesSeconds(PriorLocalTime, LocalTime);
        const auto NonContiguous = Delta > CatchupThresholdInSeconds;

        if (NonContiguous)
        {
            DeltaTime += Delta;
        }

        UpdateCurrentTemperature(Delta, NonContiguous);
        UpdateCurrentClimate(Delta, NonContiguous);
        UpdateCurrentRainfall(Delta, NonContiguous);
        // UpdateCurrentTemperature(DeltaTime, NonContiguous);
        // UpdateCurrentClimate(DeltaTime, NonContiguous);
        // UpdateCurrentRainfall(DeltaTime, NonContiguous);

        // Guard against calling this.
        // By default, it's just an O(1) lookup after UpdateCurrentTemp
        // But if Modulate is customised, it may not be cached, so this would
        // incur an unneeded penalty
        if (SunriseCallback.IsBound() || SunsetCallback.IsBound() || TwilightCallback.IsBound())
        {
            // Okay, set want to check things
            const auto SunVector = DateTimeSystem->GetSunVector(RadLatitude, RadLongitude);
            const float VectorDot = FVector::DotProduct(SunVector, FVector::UpVector);

            const auto SunIsAboveHorizonThreshold = VectorDot > SunPositionAboveHorizonThreshold;
            const auto SunIsBelowHorizonThreshold = VectorDot < -SunPositionBelowHorizonThreshold;

            if (SunIsAboveHorizonThreshold)
            {
                if (SunriseCallback.IsBound() && !SunHasRisen)
                {
                    SunriseCallback.Broadcast();
                }
                SunHasRisen = true;
                SunHasSet = false;
            }
            else if (SunIsBelowHorizonThreshold)
            {
                if (SunsetCallback.IsBound() && !SunHasSet)
                {
                    SunsetCallback.Broadcast();
                }
                SunHasRisen = false;
                SunHasSet = true;
            }
            else
            {
                if (TwilightCallback.IsBound() && (SunHasSet || SunHasRisen))
                {
                    TwilightCallback.Broadcast();
                }
                SunHasRisen = false;
                SunHasSet = false;
            }
        }

        if (UpdateLocalClimateCallback.IsBound() || UpdateLocalClimateSignal.IsBound())
        {
            // Check if DeltaTime is greater than threshold
            AccumulatedDeltaForCallback += DeltaTime;
            if (AccumulatedDeltaForCallback > OneOverUpdateFrequency)
            {
                // Update
                if (UpdateLocalClimateCallback.IsBound())
                {
                    const auto UpdatedClimateData = GetUpdatedClimateData();
                    UpdateLocalClimateCallback.Broadcast(UpdatedClimateData);
                }

                if (UpdateLocalClimateSignal.IsBound())
                {
                    UpdateLocalClimateSignal.Broadcast();
                }

                AccumulatedDeltaForCallback = 0.f;
            }
        }
    }
}

void UClimateComponent::InternalBegin()
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("InternalBegin"), STAT_ACICSInternalBegin, STATGROUP_ACIClimateSys);

    RadLatitude = FMath::DegreesToRadians(ReferenceLatitude);
    RadLongitude = FMath::DegreesToRadians(ReferenceLongitude);
    PercentileLatitude = RadLatitude * INV_PI * 2;
    PercentileLongitude = RadLongitude * INV_PI;

    OneOverUpdateFrequency = 1 / DefaultClimateUpdateFrequency;
    HourlyToPerBin = 24.f / NumberOfRainSlotsPerDay;

    // Let's go
    if (ClimateTable)
    {
        TArray<FDateTimeSystemClimateMonthlyRow *> LocalClimateBook;
        ClimateTable->GetAllRows<FDateTimeSystemClimateMonthlyRow>(FString("Climate Rows"), LocalClimateBook);

        for (const auto val : LocalClimateBook)
        {
            ClimateBook.Add(DateTimeRowHelpers::CreateClimateMonthlyFromTableRow(val));
        }
    }

    if (ClimateOverridesTable)
    {
        TArray<FDateTimeSystemClimateOverrideRow *> LocalOverrides;
        ClimateOverridesTable->GetAllRows<FDateTimeSystemClimateOverrideRow>(FString("Climate Rows"), LocalOverrides);

        for (const auto val : LocalOverrides)
        {
            auto Override = DateTimeRowHelpers::CreateClimateMonthlyOverrideFromTableRow(val);
            DateOverrides.Add(GetDateHash(Override), Override);
        }
    }

    // Try to Find DateTime
    DateTimeSystem = FindComponent();

    if (DateTimeSystem && DateTimeSystem->IsReady())
    {
        if (!HasBoundToDate && DateTimeSystem->GetCore())
        {
            DateTimeSystem->GetCore()->DateChangeCallback.AddDynamic(this, &UClimateComponent::InternalDateChanged);
            DateTimeSystem->GetCore()->CleanTimeUpdate.AddDynamic(this, &UClimateComponent::UpdateLocalTimePassthrough);
            HasBoundToDate = true;
        }

        FDateTimeSystemStruct Today;
        DateTimeSystem->GetTodaysDateTZ(Today, TimezoneInfo);

        // Set Prevalue
        CachedLowTemp.Value = GetDailyLow(Today);
        CachedLowTemp.Valid = true;

        FDateTimeSystemStruct Yesterday;
        DateTimeSystem->GetYesterdaysDateTZ(Yesterday, TimezoneInfo);
        CachedPriorDewPoint.Value = GetDailyDewPoint(Yesterday);
        CachedPriorDewPoint.Valid = true;

        DTSTimeScale = DateTimeSystem->GetTimeScale();
    }

    IsInitialised = true;
}

void UClimateComponent::SetClimateUpdateFrequency(float Frequency)
{
    if (Frequency == Frequency && Frequency > 0)
    {
        // Compute the reciprocal
        OneOverUpdateFrequency = 1 / Frequency;
    }
}

FRotator UClimateComponent::GetLocalSunRotation(FVector Location)
{
    if (DateTimeSystem)
    {
        // 1.1.1 - Compute as if X is North, then correct to northing
        const auto SunRotation = DateTimeSystem->GetLocalisedSunRotation(
            PercentileLatitude, PercentileLongitude, Location);
        return RotateByNorthing(SunRotation);
    }

    return FRotator();
}

FRotator UClimateComponent::GetLocalMoonRotation(FVector Location)
{
    if (DateTimeSystem)
    {
        // 1.1.1 - Compute as if X is North, then correct to northing
        const auto MoonRotation = DateTimeSystem->GetLocalisedMoonRotation(
            PercentileLatitude, PercentileLongitude, Location);
        return RotateByNorthing(MoonRotation);
    }

    return FRotator();
}

FMatrix UClimateComponent::GetLocalNightSkyMatrix(FVector Location)
{
    if (DateTimeSystem)
    {
        // 1.1.1 - Compute as if X is North, then correct to northing
        const auto NightMatrix =
            DateTimeSystem->GetLocalisedNightSkyRotationMatrix(PercentileLatitude, PercentileLongitude, Location);
        return RotateByNorthing(NightMatrix);
    }

    return FMatrix();
}

void UClimateComponent::DateChanged_Implementation(FDateTimeSystemStruct &DateStruct)
{
}

float UClimateComponent::GetAnalyticalHighForDate(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAnalyticalHighForDate"), STAT_ACICSGetAnalyticalHighForDate,
                                STATGROUP_ACIClimateSys);

    // We have two things to do here. Return the cache, if it's valid
    if (CachedAnalyticalMonthlyHighTemp.Valid)
    {
        return CachedAnalyticalMonthlyHighTemp.Value;
    }
    else
    {
        // We need
        if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
        {
            // Which do we need. We need the fractional month value
            const auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            const auto CurrentMonthHigh = ClimateBook[DateStruct.Month]->MonthlyHighTemp;
            const auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                const auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                const auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

                // High is lerp frac
                // BUG!
                CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
            }
            CachedAnalyticalMonthlyHighTemp.Valid = true;
            return CachedAnalyticalMonthlyHighTemp.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetAnalyticalLowForDate(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAnalyticalLowForDate"), STAT_ACICSGetAnalyticalLowForDate,
                                STATGROUP_ACIClimateSys);

    // We have two things to do here. Return the cache, if it's valid
    if (CachedAnalyticalMonthlyLowTemp.Valid)
    {
        return CachedAnalyticalMonthlyLowTemp.Value;
    }
    else
    {
        // We need
        if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
        {
            // Which do we need. We need the fractional month value
            const auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            const auto CurrentMonthLow = ClimateBook[DateStruct.Month]->MonthlyLowTemp;
            const auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                const auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                const auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
            }
            CachedAnalyticalMonthlyLowTemp.Valid = true;
            return CachedAnalyticalMonthlyLowTemp.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetAnalyticalDewPointForDate(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAnalyticalDewPointForDate"), STAT_ACICSGetAnalyticalDewPointForDate,
                                STATGROUP_ACIClimateSys);

    // We have two things to do here. Return the cache, if it's valid
    if (CachedAnalyticalDewPoint.Valid)
    {
        return CachedAnalyticalDewPoint.Value;
    }
    else
    {
        if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
        {
            // Which do we need. We need the fractional month value
            const auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            const auto CurrentRH = ClimateBook[DateStruct.Month]->DewPoint;
            const auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                const auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->DewPoint;

                // High is lerp frac
                CachedAnalyticalDewPoint.Value = FMath::Lerp(CurrentRH, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                const auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                const auto OtherValue = ClimateBook[OtherIndex]->DewPoint;

                // High is lerp frac
                CachedAnalyticalDewPoint.Value = FMath::Lerp(CurrentRH, OtherValue, BlendFrac);
            }
            CachedAnalyticalDewPoint.Valid = true;
            return CachedAnalyticalDewPoint.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetDailyHigh(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetDailyHigh"), STAT_ACICSGetDailyHigh, STATGROUP_ACIClimateSys);

    if (CachedHighTemp.Valid)
    {
        return CachedHighTemp.Value;
    }

    // Check Override
    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            CachedHighTemp.Value =
                DailyHighModulation(DateStruct, asPtr->MiscData, asPtr->HighTemp, LastLowTemp, LastHighTemp);
            CachedHighTemp.Valid = true;
        }
    }
    else
    {
        auto DummyTagContainer = FGameplayTagContainer();

        CachedHighTemp.Value = DailyHighModulation(DateStruct, DummyTagContainer, GetAnalyticalHighForDate(DateStruct),
                                                   LastLowTemp, LastHighTemp);
        CachedHighTemp.Valid = true;
    }

    return CachedHighTemp.Value;
}

float UClimateComponent::GetDailyLow(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetDailyLow"), STAT_ACICSGetDailyLow, STATGROUP_ACIClimateSys);

    // Okay. We need to work out which Low we actually want

    if (CachedNextLowTemp.Valid)
    {
        return CachedNextLowTemp.Value;
    }

    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            CachedNextLowTemp.Value =
                DailyLowModulation(DateStruct, asPtr->MiscData, asPtr->LowTemp, LastLowTemp, LastHighTemp);
            CachedNextLowTemp.Valid = true;
        }
    }
    else
    {
        auto DummyTagContainer = FGameplayTagContainer();

        CachedNextLowTemp.Value = DailyLowModulation(DateStruct, DummyTagContainer, GetAnalyticalLowForDate(DateStruct),
                                                     LastLowTemp, LastHighTemp);
        CachedNextLowTemp.Valid = true;
    }

    return CachedNextLowTemp.Value;
}

float UClimateComponent::GetPrecipitationThreshold(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetPrecipitationThreshold"), STAT_ACICSGetPrecipitationThreshold,
                                STATGROUP_ACIClimateSys);

    const auto PrecipHash = DateStruct.GetHash();

    const auto Cache = CachedProbability.Find(PrecipHash);
    if (Cache)
    {
        return *Cache;
    }

    // Compute the threshold for today's rainfall
    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            CachedProbability.Add(PrecipHash, asPtr->RainfallProbability);
            return asPtr->RainfallProbability;
        }
    }
    else
    {
        const auto AnalyticPrecip = GetAnalyticalPrecipitationThresholdDate(DateStruct);
        CachedProbability.Add(PrecipHash, AnalyticPrecip);
        return AnalyticPrecip;
    }

    return 0.0f;
}

float UClimateComponent::GetAnalyticalPrecipitationThresholdDate(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAnalyticalPrecipitationThresholdDate"),
                                STAT_ACICSGetAnalyticalPrecipitationThresholdDate, STATGROUP_ACIClimateSys);

    // We have two things to do here. Return the cache, if it's valid
    const auto PrecipHash = DateStruct.GetHash();

    const auto Cache = CachedProbability.Find(PrecipHash);
    if (Cache)
    {
        return *Cache;
    }
    else
    {
        if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
        {
            // Which do we need. We need the fractional month value
            const auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            const auto CurrentProbability = ClimateBook[DateStruct.Month]->RainfallProbability;
            const auto BlendFrac = FMath::Abs(MonthFrac - 0.5);
            int OtherIndex = 0;

            if (MonthFrac > 0.5)
            {
                // Future Month
                OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
            }
            else
            {
                // Past Month
                OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
            }

            const auto OtherValue = ClimateBook[OtherIndex]->RainfallProbability;
            const auto PrecipThresh = FMath::Lerp(CurrentProbability, OtherValue, BlendFrac);

            CachedProbability.Add(PrecipHash, PrecipThresh);

            return PrecipThresh;
        }
    }

    return 0.0f;
}

float UClimateComponent::GetRainfallAmount(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetRainfallAmount"), STAT_ACICSGetRainfallAmount, STATGROUP_ACIClimateSys);

    const auto RainfallHash = DateStruct.GetHash();

    const auto Cache = CachedRainfallLevels.Find(RainfallHash);
    if (Cache)
    {
        return *Cache;
    }

    // Compute the threshold for today's rainfall
    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            const auto RainfallByProbability = asPtr->HourlyRainfall * HourlyToPerBin * (
                                                   1 / asPtr->RainfallProbability);
            CachedRainfallLevels.Add(RainfallHash, RainfallByProbability);
            return RainfallByProbability;
        }
    }
    else
    {
        const auto Cacheable = GetAnalyticalPrecipitationAmountDate(DateStruct);
        CachedRainfallLevels.Add(RainfallHash, Cacheable);
        return Cacheable;
    }

    return 0.0f;
}

float UClimateComponent::GetAnalyticalPrecipitationAmountDate(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetAnalyticalPrecipitationAmountDate"),
                                STAT_ACICSGetAnalyticalPrecipitationAmountDate, STATGROUP_ACIClimateSys);

    const auto RainfallHash = DateStruct.GetHash();

    const auto Cache = CachedAnalyticRainfallLevel.Find(RainfallHash);
    if (Cache)
    {
        return *Cache;
    }
    else
    {
        if (DateStruct.Month < ClimateBook.Num() && DateTimeSystem)
        {
            // Which do we need. We need the fractional month value
            const auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            const auto CurrentProbability = ClimateBook[DateStruct.Month]->HourlyAverageRainfall * HourlyToPerBin *
                                            (1 / ClimateBook[DateStruct.Month]->RainfallProbability);
            const auto BlendFrac = FMath::Abs(MonthFrac - 0.5);
            int OtherIndex = 0;

            if (MonthFrac > 0.5)
            {
                // Future Month
                OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
            }
            else
            {
                // Past Month
                OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
            }

            const auto OtherValue = ClimateBook[OtherIndex]->HourlyAverageRainfall * HourlyToPerBin *
                                    (1 / ClimateBook[OtherIndex]->RainfallProbability);
            const auto ResultingPrecip = FMath::Lerp(CurrentProbability, OtherValue, BlendFrac);

            CachedAnalyticRainfallLevel.Add(RainfallHash, ResultingPrecip);

            return ResultingPrecip;
        }
    }

    return 0.0f;
}

float UClimateComponent::GetDailyDewPoint(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetDailyDewPoint"), STAT_ACICSGetDailyDewPoint, STATGROUP_ACIClimateSys);

    // Okay. We need to work out which Low we actually want

    if (CachedNextDewPoint.Valid)
    {
        return CachedNextDewPoint.Value;
    }

    const auto Row = DateOverrides.Find(GetDateHash(DateStruct));
    if (Row)
    {
        const auto asPtr = *Row;
        if (asPtr)
        {
            CachedNextDewPoint.Value = asPtr->DewPoint;
            CachedNextDewPoint.Valid = true;
        }
    }
    else
    {
        auto DummyTagContainer = FGameplayTagContainer();

        CachedNextDewPoint.Value = GetAnalyticalDewPointForDate(DateStruct);
        CachedNextDewPoint.Valid = true;
    }

    return CachedNextDewPoint.Value;
}

UClimateComponent::UClimateComponent()
{
    ClimateSetup();
}

UClimateComponent::UClimateComponent(UClimateComponent &Other)
{
    ClimateSetup();
}

UClimateComponent::UClimateComponent(const FObjectInitializer &ObjectInitializer)
{
    ClimateSetup();
}

void UClimateComponent::BeginPlay()
{
    if (TicksPerSecond > 0)
    {
        SetComponentTickInterval(1.0 / TicksPerSecond);
    }

    Super::BeginPlay();

    InternalBegin();
}

void UClimateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    InternalTick(DeltaTime);
}
