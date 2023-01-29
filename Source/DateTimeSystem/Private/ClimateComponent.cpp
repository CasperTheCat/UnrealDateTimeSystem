// [TEMPLATE_COPYRIGHT]

#include "ClimateComponent.h"
#include "GameFramework/GameState.h"
#include "Interfaces.h"

void UClimateComponent::ClimateSetup()
{
    TicksPerSecond = 4;
    // PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    RegisterAllComponentTickFunctions(true);
    HasBoundToDate = true;
    DefaultClimateUpdateFrequency = 30;
    TemperatureChangeSpeed = 0.001;
    FogChangeSpeed = 0.001;
    DTSTimeScale = 1.f;
    SunHasRisen = false;
    SunHasSet = false;

    SunPositionBelowHorizonThreshold = 0.087155f;
    SunPositionAboveHorizonThreshold = 0.0435775f;
}

void UClimateComponent::Invalidate(EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
{
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
    }
}

void UClimateComponent::UpdateLocalTimePassthrough(FDateTimeSystemStruct NewTime)
{
    FDateTimeSystemStruct Local{};

    // Check again, for consistency
    if (DateTimeSystem && IsValid(DateTimeSystem))
    {
        // Update Local Time. We need it for a few things
        DateTimeSystem->GetTodaysDateTZ(Local, TimezoneInfo);
    }

    if (UpdateLocalTime.IsBound())
    {
        UpdateLocalTime.Broadcast(Local);
    }
}

FDateTimeSystemStruct UClimateComponent::GetLocalTime()
{
    // SunHasRisen = false;
    return LocalTime;
}

void UClimateComponent::BindToDateTimeSystem()
{
    if (DateTimeSystem && IsValid(DateTimeSystem) && UpdateLocalTime.IsBound())
    {
        // If someone is listening to the update, we pass it through
        DateTimeSystem->TimeUpdate.AddDynamic(this, &UClimateComponent::UpdateLocalTimePassthrough);
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

        auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
        auto Multi = FMath::Sin(PI * FracDay);
        return FMath::Lerp(LowTemperature, HighTemperature, Multi);

        // Using a proportional control
        auto SunPower = FVector::DotProduct(SunVector, FVector::UpVector);

        auto ClampedValues = FMath::Clamp(SunPower + 0.5, 0.f, 1.f);
        auto SunTarget = FMath::Lerp(LowTemperature, HighTemperature, ClampedValues);// SunPower * 0.5 + 0.5);
        return SunTarget;

        auto SunError = (SunTarget - Temperature);

        if (FMath::Abs(SunError) > TemperatureCatchupThreshold)
        {
            //
            GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
                                             FString("Catching up to analytic.") + FString::SanitizeFloat(SunError));
            return SunTarget;
        }

        return Temperature + SunError * TemperatureChangeSpeed * SecondsSinceUpdate * DTSTimeScale;
       

        //if (FVector::DotProduct(SunVector, FVector::UpVector) > 0)
        //{
        //    // Sun is above the horizon
        //    // We want to start Lerping to high temperature slowly
        //    return FMath::FInterpTo(Temperature, HighTemperature, SecondsSinceUpdate, TemperatureChangeSpeed);
        //    //auto SunError = HighTemperature - Temperature;
        //    //return Temperature + (SunError * SunPower * TemperatureChangeSpeed);
        //}
        //else
        //{
        //    return FMath::FInterpTo(Temperature, LowTemperature, SecondsSinceUpdate, TemperatureChangeSpeed);
        //   
        //}

        // auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);
        // auto Multi = FMath::Sin(PI * FracDay);
        // return FMath::Lerp(LowTemperature, HighTemperature, Multi);
    }

    return 0.f;
}

float UClimateComponent::ModulateFogByRainfall_Implementation(float FogHeight, float SecondsSinceUpdate,
                                                              float RainLevel)
{
    return FogHeight;
}

float UClimateComponent::ModulateTemperatureByLocation(float Temperature, FVector Location)
{
    auto AltitudeAboveSeaLevel = Location.Z - SeaLevel;
    return Temperature - (AltitudeAboveSeaLevel * 0.000065);
}

FDateTimeClimateDataStruct UClimateComponent::GetUpdatedClimateData_Implementation()
{
    auto WindVector = FVector(0, 0, 0);

    FDateTimeClimateDataStruct Returnable{};
    Returnable.Temperature = GetCurrentTemperature();
    Returnable.HeatOffset = GetHeatIndex();
    Returnable.Wind = WindVector;
    Returnable.ChillOffset = GetWindChillFromVector(WindVector);
    Returnable.Frost = 0.f;
    Returnable.Rain = FMath::Sin(float(LocalTime.StoredSolarSeconds * (1.f / 86400.f) * 3.14)) * 0.5 + 0.5;
    Returnable.Wetness = FMath::Cos(float(LocalTime.StoredSolarSeconds * (1.f / 86400.f) * 6.14)) * 0.5 + 0.5;

    //
    return Returnable;
}

void UClimateComponent::UpdateCurrentTemperature(float DeltaTime)
{
    // Okay. Our Current Temp is going to be dumb.
    // Which low are we using? The first or last?
    if (DateTimeSystem)
    {
        auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);

        auto HighTemp = GetDailyHigh(LocalTime);
        auto LowTemp = CachedLowTemp.Value;
        if (FracDay > 0.5)
        {
            // Need today's low, rather than the prior
            LowTemp = GetDailyLow(LocalTime);
        }

        CurrentTemperature = ModulateTemperature(CurrentTemperature, DeltaTime, LowTemp, HighTemp);
    }
}

void UClimateComponent::UpdateCurrentClimate(float DeltaTime)
{
    if (DateTimeSystem)
    {
        // Rel. Humidity
        auto FracDay = DateTimeSystem->GetFractionalDay(LocalTime);

        //
        auto NextRH = GetDailyDewPoint(LocalTime);
        auto LowRH = CachedPriorDewPoint.Value;
        CurrentDewPoint = FMath::Lerp(LowRH, NextRH, FracDay);

        // Ru
        auto LogRH = (CurrentDewPoint * 18.678f) / (257.14f + CurrentDewPoint) -
                     (CurrentTemperature * 18.678f) / (257.14f + CurrentTemperature);

        // RH
        CurrentRelativeHumidity = FMath::Exp(LogRH);
    }
}

void UClimateComponent::InternalDateChanged(FDateTimeSystemStruct DateStruct)
{
    // Handle what was once done on InternalTick
    Invalidate(EDateTimeSystemInvalidationTypes::Day);

    // Handle rolling the starting temp of day n+1 to the ending of n
    CachedLowTemp.Value = CachedNextLowTemp.Value;
    CachedLowTemp.Valid = true;
    LastHighTemp = CachedHighTemp.Value;
    LastLowTemp = CachedLowTemp.Value;
    CachedPriorDewPoint.Value = CachedNextDewPoint.Value;

    auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
    if (Row)
    {
        auto asPtr = *Row;
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

UDateTimeSystemComponent *UClimateComponent::FindComponent()
{
    auto World = GetWorld();
    if (World)
    {
        auto GameInst = World->GetGameInstance();
        if (GameInst)
        {
            // Try to get from here
            auto AsType = Cast<IDateTimeSystemInterface>(GameInst);
            if (AsType)
            {
                //
                return IDateTimeSystemInterface::Execute_GetDateTimeSystem(GameInst);
            }
        }

        // If we're here, Instance failed to get the time system.
        auto GameState = World->GetGameState();
        if (GameState)
        {
            auto AsType = Cast<IDateTimeSystemInterface>(GameState);
            if (AsType)
            {
                return IDateTimeSystemInterface::Execute_GetDateTimeSystem(GameState);
                // return AsType->GetDateTimeSystem();
            }
        }
    }

    return nullptr;
}

float UClimateComponent::GetCurrentTemperature()
{
    return CurrentTemperature;
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
    auto AltitudeAboveSeaLevel = Location.Z - SeaLevel;
    return CurrentDewPoint - (AltitudeAboveSeaLevel * 0.000018);
}

float UClimateComponent::GetRelativeHumidityForLocation(FVector Location)
{
    auto GetLocalDewPoint = GetCurrentDewPointForLocation(Location);


    auto LogRH = (GetLocalDewPoint * 18.678f) / (257.14f + GetLocalDewPoint) -
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
    auto CloudHeightMetres = FMath::Abs(CurrentTemperature - CurrentDewPoint) * (1000 / 6.5f);

    return (CloudHeightMetres * 100);
}

float UClimateComponent::GetFogLevel(float DeltaTime, float Scale)
{
    // Fog is strange
    // Fog forms when Delta(Td, T) is less than 2.5C

    auto Delta = FMath::Abs(CurrentTemperature - CurrentDewPoint);
    auto TargetFogLevel = 0.f;

    auto TargetMultiplier = FMath::Clamp((Delta - 2.f) * 2, 0.f, 1.f);

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

    auto TFL = (TargetFogLevel - CurrentFog) * FogChangeSpeed * DeltaTime * DTSTimeScale;
    CurrentFog = CurrentFog + TFL;

    return FMath::Abs(TargetFogLevel) * Scale * 100 + SeaLevel;

    checkNoEntry();
    auto CL = -GetCloudLevel();
    return ModulateFogByRainfall(CL, 0, 0);
}

float UClimateComponent::GetHeatIndex()
{
    auto HeatIndex = GetHeatIndexForLocation(FVector(0,0,SeaLevel));
    //auto T = CurrentTemperature;
    //auto R = CurrentRelativeHumidity;
    //auto TT = T * T;
    //auto RR = R * R;

    //// Constants
    //auto C1 = -8.78469475556f;
    //auto C2 = 1.61139411f;
    //auto C3 = 2.33854883889f;
    //auto C4 = -0.14611605f;
    //auto C5 = -0.012308084f;
    //auto C6 = -0.0164248277778f;
    //auto C7 = 2.211732e-3f;
    //auto C8 = 7.2546e-4f;
    //auto C9 = -3.582e-6f;

    //auto HeatIndex = C1 + C2 * T + C3 * R + C4 * T * R + C5 * TT + C6 * RR + C7 * TT * R + C8 * T * RR + C9 * TT * RR;

    // We want to mute HI when temp is under 25C
    // We can do this rolling off
    auto HeatIndexDelta = HeatIndex - CurrentTemperature;

    return FMath::Lerp(0, HeatIndexDelta, FMath::Clamp(CurrentTemperature - 25, 0, 1));
}

float UClimateComponent::GetHeatIndexForLocation(FVector Location)
{
    //auto RelativeMSL(Location.Z - SeaLevel);
    auto T = GetCurrentTemperatureForLocation(Location);
//    auto R = CurrentRelativeHumidity;
    auto R = GetRelativeHumidityForLocation(Location);
    auto TT = T * T;
    auto RR = R * R;

    // Constants
    auto C1 = -8.78469475556f;
    auto C2 = 1.61139411f;
    auto C3 = 2.33854883889f;
    auto C4 = -0.14611605f;
    auto C5 = -0.012308084f;
    auto C6 = -0.0164248277778f;
    auto C7 = 2.211732e-3f;
    auto C8 = 7.2546e-4f;
    auto C9 = -3.582e-6f;

    auto HeatIndex = C1 + C2 * T + C3 * R + C4 * T * R + C5 * TT + C6 * RR + C7 * TT * R + C8 * T * RR + C9 * TT * RR;

    return HeatIndex;

    //// We want to mute HI when temp is under 25C
    //// We can do this rolling off
    //auto HeatIndexDelta = HeatIndex - CurrentTemperature;

    //return FMath::Lerp(0, HeatIndexDelta, FMath::Clamp(CurrentTemperature - 25, 0, 1));
}

float UClimateComponent::GetWindChillFromVector(FVector WindVector)
{
    return GetWindChillFromVelocity(WindVector.Length());
}

float UClimateComponent::GetWindChillFromVelocity(float WindVelocity)
{
    auto T = CurrentTemperature;
    auto V = FMath::Pow(WindVelocity, 0.16);

    auto WC = 13.12 + 0.6215 * T - 11.37 * V + 0.3965 * T * V;

    return FMath::Min(0, WC - CurrentTemperature);
}

// Start a counter here so it captures the super call
// DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetCameraView (Including Super::)"), STAT_ACIGetCameraViewInc,
// STATGROUP_ACIExtCam);

void UClimateComponent::InternalTick(float DeltaTime)
{
    Invalidate(EDateTimeSystemInvalidationTypes::Frame);

    if (DateTimeSystem)
    {
        // Update Local Time. We need it for a few things
        DateTimeSystem->GetTodaysDateTZ(LocalTime, TimezoneInfo);

        UpdateCurrentTemperature(DeltaTime);
        UpdateCurrentClimate(DeltaTime);

        // Guard against calling this.
        // By default, it's just an O(1) lookup after UpdateCurrentTemp
        // But if Modulate is customised, it may not be cached, so this would
        // incur an unneeded penalty
        if (SunriseCallback.IsBound() || SunsetCallback.IsBound() || TwilightCallback.IsBound())
        {
            // Okay, set want to check things
            auto SunVector = DateTimeSystem->GetSunVector(RadLatitude, RadLongitude);
            //auto SunZenith = -SunVector.ToOrientationRotator().Pitch;

            // Sunrise is 5deg under the horizon
            // UPDATE: replaced -0.087155f with 0.01f
            float VectorDot = FVector::DotProduct(SunVector, FVector::UpVector);
            //GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, SunVector.ToOrientationRotator().ToString());
            //GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::SanitizeFloat(SunZenith));
            //GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, SunVector.ToString());
            //GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::SanitizeFloat(VectorDot));
            auto SunIsAboveHorizonThreshold = VectorDot > SunPositionAboveHorizonThreshold;
            auto SunIsBelowHorizonThreshold = VectorDot < -SunPositionBelowHorizonThreshold;

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

        if (UpdateLocalClimateCallback.IsBound())
        {
            // Check if DeltaTime is greater than threshold
            AccumulatedDeltaForCallback += DeltaTime;
            if (AccumulatedDeltaForCallback > OneOverUpdateFrequency)
            {
                // Update
                auto UpdatedClimateData = GetUpdatedClimateData();
                UpdateLocalClimateCallback.Broadcast(UpdatedClimateData);
                AccumulatedDeltaForCallback = 0.f;
            }
        }
    }
}

void UClimateComponent::InternalBegin()
{
    RadLatitude = FMath::DegreesToRadians(ReferenceLatitude);
    RadLongitude = FMath::DegreesToRadians(ReferenceLongitude);
    PercentileLatitude = RadLatitude * INV_PI * 2;
    PercentileLongitude = RadLongitude * INV_PI;

    OneOverUpdateFrequency = 1 / DefaultClimateUpdateFrequency;

    // Let's go
    if (ClimateTable)
    {
        ClimateTable->GetAllRows<FDateTimeSystemClimateMonthlyRow>(FString("Climate Rows"), ClimateBook);
    }

    if (ClimateOverridesTable)
    {
        ClimateOverridesTable->GetAllRows<FDateTimeSystemClimateOverrideRow>(FString("Climate Rows"), DOTemps);

        for (auto val : DOTemps)
        {
            DateOverrides.Add(GetTypeHash(val), val);
        }
    }

    // Try to Find DateTime
    DateTimeSystem = FindComponent();

    if (DateTimeSystem && IsValid(DateTimeSystem))
    {
        if (!HasBoundToDate)
        {
            DateTimeSystem->DateChangeCallback.AddDynamic(this, &UClimateComponent::InternalDateChanged);
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

        DTSTimeScale = DateTimeSystem->TimeScale;

        if (DateTimeSystem && IsValid(DateTimeSystem) && UpdateLocalTime.IsBound())
        {
            // If someone is listening to the update, we pass it through
            DateTimeSystem->TimeUpdate.AddDynamic(this, &UClimateComponent::UpdateLocalTimePassthrough);
        }
    }
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
        return DateTimeSystem->GetLocalisedSunRotation(PercentileLatitude, PercentileLongitude, Location);
    }

    return FRotator();
}

FRotator UClimateComponent::GetLocalMoonRotation(FVector Location)
{
    if (DateTimeSystem)
    {
        return DateTimeSystem->GetLocalisedMoonRotation(PercentileLatitude, PercentileLongitude, Location);
    }

    return FRotator();
}

void UClimateComponent::DateChanged_Implementation(FDateTimeSystemStruct &DateStruct)
{
}

float UClimateComponent::GetAnalyticalHighForDate(FDateTimeSystemStruct &DateStruct)
{
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
            auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            auto CurrentMonthHigh = ClimateBook[DateStruct.Month]->MonthlyHighTemp;
            auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->MonthlyHighTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(OtherValue, CurrentMonthHigh, BlendFrac);
            }
            CachedAnalyticalMonthlyHighTemp.Valid = true;
            return CachedAnalyticalMonthlyHighTemp.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetAnalyticalLowForDate(FDateTimeSystemStruct &DateStruct)
{
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
            auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            auto CurrentMonthLow = ClimateBook[DateStruct.Month]->MonthlyLowTemp;
            auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->MonthlyLowTemp;

                // High is lerp frac
                CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(OtherValue, CurrentMonthLow, BlendFrac);
            }
            CachedAnalyticalMonthlyLowTemp.Valid = true;
            return CachedAnalyticalMonthlyLowTemp.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetAnalyticalDewPointForDate(FDateTimeSystemStruct &DateStruct)
{
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
            auto MonthFrac = DateTimeSystem->GetFractionalMonth(DateStruct);
            auto CurrentRH = ClimateBook[DateStruct.Month]->DewPoint;
            auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

            if (MonthFrac > 0.5)
            {
                // Future Month
                auto OtherIndex = (DateStruct.Month + 1) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->DewPoint;

                // High is lerp frac
                CachedAnalyticalDewPoint.Value = FMath::Lerp(CurrentRH, OtherValue, BlendFrac);
            }
            else
            {
                // Future Month
                auto OtherIndex = (ClimateBook.Num() + (DateStruct.Month - 1)) % ClimateBook.Num();
                auto OtherValue = ClimateBook[OtherIndex]->DewPoint;

                // High is lerp frac
                CachedAnalyticalDewPoint.Value = FMath::Lerp(OtherValue, CurrentRH, BlendFrac);
            }
            CachedAnalyticalDewPoint.Valid = true;
            return CachedAnalyticalDewPoint.Value;
        }
    }
    return 0.0f;
}

float UClimateComponent::GetDailyHigh(FDateTimeSystemStruct &DateStruct)
{
    if (CachedHighTemp.Valid)
    {
        return CachedHighTemp.Value;
    }

    // Check Override
    auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
    if (Row)
    {
        auto asPtr = *Row;
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
    // Okay. We need to work out which Low we actually want

    if (CachedNextLowTemp.Valid)
    {
        return CachedNextLowTemp.Value;
    }

    auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
    if (Row)
    {
        auto asPtr = *Row;
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

float UClimateComponent::GetDailyDewPoint(FDateTimeSystemStruct &DateStruct)
{
    // Okay. We need to work out which Low we actually want

    if (CachedNextDewPoint.Valid)
    {
        return CachedNextDewPoint.Value;
    }

    auto Row = DateOverrides.Find(GetTypeHash(DateStruct));
    if (Row)
    {
        auto asPtr = *Row;
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
    Super::BeginPlay();

    InternalBegin();
}

void UClimateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    InternalTick(DeltaTime);
}
