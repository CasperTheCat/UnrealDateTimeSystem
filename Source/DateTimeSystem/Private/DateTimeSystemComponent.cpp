// [TEMPLATE_COPYRIGHT]

#include "DateTimeSystemComponent.h"

UDateTimeSystemComponent::UDateTimeSystemComponent()
{
    DateTimeSetup();
}

UDateTimeSystemComponent::UDateTimeSystemComponent(UDateTimeSystemComponent &Other)
{
    DateTimeSetup();
}

UDateTimeSystemComponent::UDateTimeSystemComponent(const FObjectInitializer &ObjectInitializer)
{
    DateTimeSetup();
}

void UDateTimeSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (TimeScale > 0.f)
    {
        InternalTick(DeltaTime);
    }
}

void UDateTimeSystemComponent::BeginPlay()
{
    if (TicksPerSecond > 0)
    {
        SetComponentTickInterval(1.0 / TicksPerSecond);
    }

    Super::BeginPlay();

    InternalBegin();
}

bool UDateTimeSystemComponent::IsReady()
{
    return IsValid(CoreObject);
}

void UDateTimeSystemComponent::InternalTick(float DeltaTime, bool NonContiguous)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        // Unguared dereference in shipping build
        CoreObject->InternalTick(DeltaTime * TimeScale, NonContiguous);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }
#endif // DATETIMESYSTEM_POINTERCHECK
}

void UDateTimeSystemComponent::InternalBegin()
{
    // Create the Core
    CoreObject = NewObject<UDateTimeSystemCore>(GetTransientPackage(), CoreClass.Get());

    if (IsValid(CoreObject))
    {
        FDateTimeCommonCoreInitializer CoreInitializer{};
        CoreInitializer.LengthOfDay = LengthOfDay;
        CoreInitializer.DaysInOrbitalYear = DaysInOrbitalYear;
        CoreInitializer.YearbookTable = YearBookTable;
        CoreInitializer.DateOverridesTable = DateOverridesTable;
        CoreInitializer.UseDayIndexForOverride = UseDayIndexForOverride;
        CoreInitializer.PlanetRadius = PlanetRadius;
        CoreInitializer.ReferenceLatitude = ReferenceLatitude;
        CoreInitializer.ReferenceLongitude = ReferenceLongitude;
        CoreInitializer.StartDate = InternalDate;
        CoreInitializer.DaysInWeek = DaysInWeek;
        CoreInitializer.OverridedDatesSetDate = OverridedDatesSetDate;

        CoreObject->InternalBegin(CoreInitializer);
    }
}

void UDateTimeSystemComponent::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
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

void UDateTimeSystemComponent::GetTodaysDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
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

void UDateTimeSystemComponent::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
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

void UDateTimeSystemComponent::GetTomorrowsDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
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

void UDateTimeSystemComponent::GetYesterdaysDate(FDateTimeSystemStruct &DateStruct)
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

void UDateTimeSystemComponent::GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
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

FRotator UDateTimeSystemComponent::GetSunRotationForLocation(FVector Location)
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

FRotator UDateTimeSystemComponent::GetSunRotation()
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

FVector UDateTimeSystemComponent::GetSunVector(float Latitude, float Longitude)
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

FRotator UDateTimeSystemComponent::GetMoonRotationForLocation(FVector Location)
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

FRotator UDateTimeSystemComponent::GetMoonRotation()
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

FVector UDateTimeSystemComponent::GetMoonVector(float Latitude, float Longitude)
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

FRotator UDateTimeSystemComponent::GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
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

FRotator UDateTimeSystemComponent::GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
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

bool UDateTimeSystemComponent::DoesYearLeap(int Year)
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

FText UDateTimeSystemComponent::GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemComponent::GetTimeScale()
{
    return TimeScale;
}

void UDateTimeSystemComponent::SetTimeScale(float NewScale)
{
    TimeScale = NewScale;
}

UDateTimeSystemCore *UDateTimeSystemComponent::GetCore()
{
    return CoreObject;
}

float UDateTimeSystemComponent::GetLengthOfDay()
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

void UDateTimeSystemComponent::SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation)
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

FDateTimeSystemStruct UDateTimeSystemComponent::GetUTCDateTime()
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

void UDateTimeSystemComponent::AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
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

bool UDateTimeSystemComponent::AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety)
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

float UDateTimeSystemComponent::ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &Date1,
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

float UDateTimeSystemComponent::ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &Date1,
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

float UDateTimeSystemComponent::ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &Date1,
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

float UDateTimeSystemComponent::ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &Date1,
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

double UDateTimeSystemComponent::DComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                                  UPARAM(ref) FDateTimeSystemStruct &Date2)
{
#if DATETIMESYSTEM_POINTERCHECK
    if (IsValid(CoreObject))
    {
#endif // DATETIMESYSTEM_POINTERCHECK

        return CoreObject->DComputeDeltaBetweenDatesSeconds(Date1, Date2);

#if DATETIMESYSTEM_POINTERCHECK
    }
    else
    {
        checkNoEntry();
    }

    return 0;
#endif // DATETIMESYSTEM_POINTERCHECK
}

TTuple<float, float, float> UDateTimeSystemComponent::ComputeDeltaBetweenDatesInternal(
    UPARAM(ref) FDateTimeSystemStruct &Date1, UPARAM(ref) FDateTimeSystemStruct &Date2, FDateTimeSystemStruct &Result)
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

void UDateTimeSystemComponent::AddDateStruct(FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemComponent::GetFractionalDay(FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemComponent::GetFractionalMonth(FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemComponent::GetFractionalOrbitalYear(FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemComponent::GetFractionalCalendarYear(FDateTimeSystemStruct &DateStruct)
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

int UDateTimeSystemComponent::GetLengthOfCalendarYear(int Year)
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

int UDateTimeSystemComponent::GetDaysInCurrentMonth()
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

int UDateTimeSystemComponent::GetDaysInMonth(int MonthIndex)
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

int UDateTimeSystemComponent::GetMonthsInYear(int YearIndex)
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

bool UDateTimeSystemComponent::SanitiseDateTime(FDateTimeSystemStruct &DateStruct)
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

FVector UDateTimeSystemComponent::AlignWorldLocationInternalCoordinates(FVector WorldLocation,
                                                                        FVector NorthingDirection)
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

void UDateTimeSystemComponent::DateTimeSetup()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    RegisterAllComponentTickFunctions(true);

    LengthOfDay = 86400;
    DaysInOrbitalYear = 365.25f;
    PlanetRadius = 6360.f;
    DaysInWeek = 7;
    CoreClass = UDateTimeSystemCore::StaticClass();
    TicksPerSecond = 10.f;
    TimeScale = 1.f;
}
