// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "DateTimeSubsystem.h"
#include "DateTimeSystem/Private/DateTimeSystemSettings.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"

namespace DateTimeCVars
{
static float TimeScale = 1.f;
static FAutoConsoleVariableRef CVarTimeScale(TEXT("DateTimeSystem.TimeScale"), TimeScale,
                                             TEXT("Multiplier applied to DeltaTime"));

static int TickStride = 1;
static FAutoConsoleVariableRef CVarTickStride(TEXT("DateTimeSystem.TickStride"), TickStride,
                                              TEXT("Number of frames to stride before passing tick"));
} // namespace DateTimeCVars

UDateTimeSystem::UDateTimeSystem()
{
}

UDateTimeSystem::UDateTimeSystem(UDateTimeSystem &Other)
{
}

UDateTimeSystem::UDateTimeSystem(const FObjectInitializer &ObjectInitializer)
{
}

UDateTimeSystemCore *UDateTimeSystem::GetCore()
{
    return CoreObject;
}

bool UDateTimeSystem::IsReady()
{
    return IsValid(CoreObject);
}

void UDateTimeSystem::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetTodaysDate(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::GetTodaysDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                      FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetTodaysDateTZ(DateStruct, TimezoneInfo);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetTomorrowsDate(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::GetTomorrowsDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                         FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetTomorrowsDateTZ(DateStruct, TimezoneInfo);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::GetYesterdaysDate(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetYesterdaysDate(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                          UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->GetYesterdaysDateTZ(DateStruct, TimezoneInfo);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetSunRotationForLocation(FVector Location)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetSunRotationForLocation(Location);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetSunRotation()
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetSunRotation();

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetSunRotationForLatLong(double Latitude, double Longitude)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetSunRotationForLatLong(Latitude, Longitude);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FVector UDateTimeSystem::GetSunVector(float Latitude, float Longitude)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetSunVector(Latitude, Longitude);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FVector();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetMoonRotationForLocation(FVector Location)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetMoonRotationForLocation(Location);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetMoonRotation()
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetMoonRotation();

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetMoonRotationForLatLong(double Latitude, double Longitude)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetMoonRotationForLatLong(Latitude, Longitude);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FVector UDateTimeSystem::GetMoonVector(float Latitude, float Longitude)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetMoonVector(Latitude, Longitude);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FVector();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FText UDateTimeSystem::GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetNameOfMonth(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FText();
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::GetTimeScale()
{
    return DateTimeCVars::TimeScale;
}

float UDateTimeSystem::GetLengthOfDay()
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetLengthOfDay();

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::InternalTick(float DeltaTime, bool NonContiguous)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->InternalTick(DeltaTime * DateTimeCVars::TimeScale, NonContiguous);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        CoreObject->SetUTCDateTime(DateStruct, SkipInitialisation);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

FDateTimeSystemStruct UDateTimeSystem::GetUTCDateTime()
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetUTCDateTime();

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FDateTimeSystemStruct();
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->AdvanceToTime(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

bool UDateTimeSystem::AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->AdvanceToClockTime(Hour, Minute, Second, Safety);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return false;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDates(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                     UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDatesYears(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                      UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDatesMonths(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                    UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDatesDays(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

double UDateTimeSystem::ComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                        UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDatesSeconds(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

TTuple<float, float, float> UDateTimeSystem::ComputeDeltaBetweenDatesInternal(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                              UPARAM(ref) FDateTimeSystemStruct &Date2,
                                                                              FDateTimeSystemStruct &Result)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->ComputeDeltaBetweenDatesInternal(Date1, Date2, Result);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return TTuple<float, float, float>();
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::AddDateStruct(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->AddDateStruct(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::GetFractionalDay(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetFractionalDay(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::GetFractionalMonth(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetFractionalMonth(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::GetFractionalOrbitalYear(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetFractionalOrbitalYear(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

float UDateTimeSystem::GetFractionalCalendarYear(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetFractionalCalendarYear(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0.f;
#endif // DATETIMESYSTEM_POINTERCHECK
}

int UDateTimeSystem::GetLengthOfCalendarYear(int Year)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetLengthOfCalendarYear(Year);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

int UDateTimeSystem::GetDaysInCurrentMonth()
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetDaysInCurrentMonth();

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

int UDateTimeSystem::GetDaysInMonth(int MonthIndex)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetDaysInMonth(MonthIndex);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

int UDateTimeSystem::GetMonthsInYear(int YearIndex)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetMonthsInYear(YearIndex);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

bool UDateTimeSystem::SanitiseDateTime(FDateTimeSystemStruct &DateStruct)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->SanitiseDateTime(DateStruct);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return false;
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                  FVector Location)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetLocalisedSunRotation(BaseLatitudePercent, BaseLongitudePercent, Location);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                   FVector Location)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->GetLocalisedMoonRotation(BaseLatitudePercent, BaseLongitudePercent, Location);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

bool UDateTimeSystem::DoesYearLeap(int Year)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->DoesYearLeap(Year);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return false;
#endif // DATETIMESYSTEM_POINTERCHECK
}

FVector UDateTimeSystem::AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->AlignWorldLocationInternalCoordinates(WorldLocation, NorthingDirection);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FVector();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FVector UDateTimeSystem::RotateLocationByNorthing(FVector Location, FVector NorthingDirection)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->RotateLocationByNorthing(Location, NorthingDirection);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FVector();
#endif // DATETIMESYSTEM_POINTERCHECK
}

FRotator UDateTimeSystem::RotateRotationByNorthing(FRotator Rotation, FVector NorthingDirection)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->RotateRotationByNorthing(Rotation, NorthingDirection);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return FRotator();
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystem::Initialize(FSubsystemCollectionBase &Collection)
{
    const UGameInstance *LocalGameInstance = GetGameInstance();
    check(LocalGameInstance);

    // Create Object
    const UDateTimeSystemSettings *Settings = GetDefault<UDateTimeSystemSettings>();
    CoreObject = NewObject<UDateTimeSystemCore>((UObject *)GetTransientPackage(), Settings->CoreClass.Get());
    // CoreObject = NewObject<UDateTimeSystemCore>(this, Settings->CoreClass.Get());
    CanTick = Settings->CanEverTick;

    if (IsValid(CoreObject))
    {
        auto LocalYearBookTableObj = Settings->YearBookTable.TryLoad();
        auto LocalDateOverridesTableObj = Settings->DateOverridesTable.TryLoad();

        auto LocalYearBookTable = Cast<UDataTable>(LocalYearBookTableObj);
        auto LocalDateOverridesTable = Cast<UDataTable>(LocalDateOverridesTableObj);

        FDateTimeCommonCoreInitializer CoreInitializer{};
        CoreInitializer.LengthOfDay = Settings->LengthOfDay;
        CoreInitializer.DaysInOrbitalYear = Settings->DaysInOrbitalYear;
        CoreInitializer.YearbookTable = LocalYearBookTable;
        CoreInitializer.DateOverridesTable = LocalDateOverridesTable;
        CoreInitializer.UseDayIndexForOverride = Settings->UseDayIndexForOverride;
        CoreInitializer.PlanetRadius = Settings->PlanetRadius;
        CoreInitializer.ReferenceLatitude = Settings->ReferenceLatitude;
        CoreInitializer.ReferenceLongitude = Settings->ReferenceLongitude;
        CoreInitializer.StartDate = Settings->StartDate;
        CoreInitializer.DaysInWeek = Settings->DaysInWeek;
        CoreInitializer.OverridedDatesSetDate = Settings->OverridedDatesSetDate;

        CoreObject->InternalBegin(CoreInitializer);
    }
}

void UDateTimeSystem::Deinitialize()
{
}

bool UDateTimeSystem::ShouldCreateSubsystem(UObject *Outer) const
{
    return true;
}

void UDateTimeSystem::Tick(float DeltaTime)
{
    if (DateTimeCVars::TimeScale > 0.f && CanTick)
    {
        StoredDeltaTime += DeltaTime;
        ++CurrentTickIndex;

        // Should Tick?
        if (CurrentTickIndex > DateTimeCVars::TickStride)
        {
            InternalTick(StoredDeltaTime);
            CurrentTickIndex = 0;
            StoredDeltaTime = 0.f;
        }
    }
}

ETickableTickType UDateTimeSystem::GetTickableTickType() const
{
    return ETickableTickType();
}

bool UDateTimeSystem::IsTickable() const
{
    // TODO: Check this
    return !HasAnyFlags(RF_ClassDefaultObject) && CanTick;
}

TStatId UDateTimeSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UDateTimeSystem, STATGROUP_Tickables);
}

UWorld *UDateTimeSystem::GetTickableGameObjectWorld() const
{
    return GetGameInstance()->GetWorld();
}
