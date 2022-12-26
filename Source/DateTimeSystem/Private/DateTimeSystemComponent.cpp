// [TEMPLATE_COPYRIGHT]

#include "DateTimeSystemComponent.h"

float HelperMod(double X, double Y)
{
    const double AbsY = FMath::Abs(Y);
    if (AbsY <= 1.e-8)
    {
        return 0.0;
    }

    const double Div = (X / Y);
    double Frac = FMath::Fractional(Div);

    if (Frac < 0)
    {
        Frac += 1;
    }

    const double Result = Y * Frac;
    return Result;
}

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
    // SetComponentTickInterval(1.0 / TicksPerSecond);
    Super::BeginPlay();

    InternalBegin();
}

void UDateTimeSystemComponent::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds;
    DateStruct.Day = InternalDate.Day;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;
}

void UDateTimeSystemComponent::GetTodaysDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                               FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds + TimezoneInfo.HoursDeltaFromMeridian * 3600;
    DateStruct.Day = InternalDate.Day;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Day = InternalDate.Day + 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetTomorrowsDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                                  FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds + TimezoneInfo.HoursDeltaFromMeridian * 3600;
    HandleDayRollover(DateStruct);

    DateStruct.Day = InternalDate.Day + 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetYesterdaysDate(FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Day = InternalDate.Day - 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                                   UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds + TimezoneInfo.HoursDeltaFromMeridian * 3600;
    HandleDayRollover(DateStruct);

    DateStruct.Day = InternalDate.Day - 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

float UDateTimeSystemComponent::GetLatitudeFromLocation_Implementation(float BaseLatitudePercent, FVector Location)
{
    auto LatLoc = Location.X * 0.01 * InvPlanetRadius * INV_PI * 2; // Divide distance walked by Pi/2
    LatLoc += BaseLatitudePercent;
    auto ScaledX = LatLoc - 1;               // Shift range to from -1 to 1, to -2 to 0
    auto Modded = HelperMod(ScaledX, 4) - 2; // Take the modulus, which now goes from -2 to 2 when x is -3 to 1
    auto Absolute = FMath::Abs(Modded) - 1;  // Invert the negatives and give use a triangle wave

    return HALF_PI * Absolute;
}

float UDateTimeSystemComponent::GetLongitudeFromLocation_Implementation(float BaseLatitudePercent,
                                                                        float BaseLongitudePercent, FVector Location)
{
    // auto
    auto LatLoc = Location.X * 0.01 * InvPlanetRadius * INV_PI * 2; // Divide distance walked by Pi/2
    auto LongLoc = Location.Y * 0.01 * InvPlanetRadius * INV_PI;

    LatLoc += BaseLatitudePercent;
    LongLoc += BaseLongitudePercent;

    auto Flipped = HelperMod((LatLoc - 1) * 0.5, 2) < 1;
    if (Flipped)
    {
        LongLoc += 1; // Flip into the other hemisphere
    }

    auto ScaledX = LongLoc;
    auto Modded = HelperMod(ScaledX, 2);
    // auto Absolute = FMath::Abs(Modded) - 1; // Radians

    // FMath::Wrap(ScaledX, 0.0, 4.0);

    return PI * Modded;
}

bool UDateTimeSystemComponent::DoesYearLeap_Implementation(int Year)
{
    return false;
}

FRotator UDateTimeSystemComponent::GetSunRotationForLocation_Implementation(FVector Location)
{
    return GetLocalisedSunRotation(PercentLatitude, PercentLongitude, Location);
}

FRotator UDateTimeSystemComponent::GetSunRotation_Implementation()
{
    return GetSunRotationForLocation(FVector::ZeroVector);
}

FVector UDateTimeSystemComponent::GetSunVector_Implementation(float Latitude, float Longitude)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetSunVector_Implementation"), STAT_ACIGetSunVector, STATGROUP_ACIDateTimeSys);

    // Check Cache. We may compute this a few times per frame
    auto HashType = HashCombine(GetTypeHash(Latitude), GetTypeHash(Longitude));
    auto Cache = CachedSunVectors.Find(HashType);
    if (Cache)
    {
        return *Cache;
    }

    // Okay, compute this one

    // First, we want Solar Elevation. This is our Pitch component
    // Then we want to get our yaw, which is related to Solar Hour

    // https://gml.noaa.gov/grad/solcalc/solareqns.PDF

    float LocalLat = Latitude;
    float LocalLong = Longitude;
    float YearInRads = GetSolarFractionalYear();
    // float DeclAngle = GetDeclinationAngle(InternalDate); // This is a hack. We can use SolarDecl... for a better one
    float DeclAngle = SolarDeclinationAngle(YearInRads);

    // Correct to meridian solar time
    // float SolarHour = InternalDate.Seconds - TimezoneInfo.HoursDeltaFromMeridian * 3600;
    float EQTime = SolarTimeCorrection(YearInRads);

    // TimeOffset is acting
    float TimeOffset = EQTime + 4 * FMath::RadiansToDegrees(LocalLong); // -60 * TimezoneInfo.HoursDeltaFromMeridian;

    // Get the time in minutes, and add the offset, also in minutes
    float CorrectedTime = InternalDate.GetTimeInMinutes() + TimeOffset;
    float SolarHour = (CorrectedTime / 4) - 180;
    float SolarAngle = FMath::DegreesToRadians(SolarHour);

    // New Method
    float LatOut = DeclAngle;
    float LongOut = FMath::DegreesToRadians(-15 * (((InternalDate.Seconds - LengthOfDay * 0.5) + EQTime * 60) / 3600));
    float LongDiff = LongOut - LocalLong;
    float SX = FMath::Cos(LatOut) * FMath::Sin(LongDiff);
    float SY =
        FMath::Cos(LocalLat) * FMath::Sin(LatOut) - FMath::Sin(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);
    float SZ =
        FMath::Sin(LocalLat) * FMath::Sin(LatOut) + FMath::Cos(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);

    auto SunInverse = FVector(SY, SX, SZ).GetSafeNormal();

    CachedSunVectors.Add(HashType, SunInverse);

    return SunInverse;

    //// Time for Zenith
    // float SolarZenith = FMath::Sin(LocalLat) * FMath::Sin(DeclAngle) +
    //	FMath::Cos(LocalLat) * FMath::Cos(DeclAngle) * FMath::Cos(SolarAngle);
    // float SolarElevation = FMath::Acos(SolarZenith);

    // float SolarAzimuth = (
    //	(FMath::Sin(DeclAngle) - FMath::Sin(LocalLat) * SolarZenith)
    //	/ FMath::Cos(LocalLat) * FMath::Sin(SolarElevation));
    // float SolarYaw = FMath::RadiansToDegrees(FMath::Acos(SolarAzimuth));

    // return FRotator(-90 - FMath::RadiansToDegrees(SolarElevation), 180 - SolarYaw, 0);
}

FRotator UDateTimeSystemComponent::GetMoonRotationForLocation_Implementation(FVector Location)
{
    return GetLocalisedMoonRotation(PercentLatitude, PercentLongitude, Location);
}

FRotator UDateTimeSystemComponent::GetMoonRotation_Implementation()
{
    return GetMoonRotationForLocation(FVector::ZeroVector);
}

/////
// https://www.nrel.gov/docs/fy10osti/47681.pdf
FVector UDateTimeSystemComponent::GetMoonVector_Implementation(float Latitude, float Longitude)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetMoonVector_Implementation"), STAT_ACIGetMoonVector, STATGROUP_ACIDateTimeSys);

    double Day = GetJulianDay(InternalDate);
    double T = Day * 0.00273785078713210130047912388775;

    auto H = InternalDate.StoredSolarSeconds * InvLengthOfDay;
    auto SiderealTimeRads = 100.4606184 + 0.9856473662862 * Day + 15 * H + T * T;

    double GLong = 218.3164477 + 4812.6788123421 * T - 0.00000015786 * T * T; // +
                                                                              // 0.000000000001855835 * T * T * T -
    // 0.00000000000000015338834 * T * T * T * T;

    double GLat = 93.2720950 + 4832.020175233 * T - 0.00000036539 * T * T; // -
    // 0.00000000000028360748723766307 * T * T * T +
    // 0.0000000000000000115833246458398 * T * T * T * T;

    GLong = HelperMod(GLong, 360.f);
    GLat = HelperMod(GLat, 360.f);

    // auto DeclAngle = FMath::Asin(
    //	FMath::Sin(Beta) * FMath::Cos(Epsilon) +
    //	FMath::Cos(Beta) * FMath::Sin(Epsilon) * FMath::Sin(Lambda)
    //);

    // GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::SanitizeFloat(GLong));
    // GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Magenta, FString::SanitizeFloat(GLat));

    double LocalLat = Latitude;
    double LocalLong = Longitude;
    double LatOut = FMath::DegreesToRadians(GLat);
    double LongOut = FMath::DegreesToRadians(GLong);
    double LongDiff = LongOut - LocalLong;
    double SX = FMath::Cos(LatOut) * FMath::Sin(LongDiff);
    double SY =
        FMath::Cos(LocalLat) * FMath::Sin(LatOut) - FMath::Sin(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);
    double SZ =
        FMath::Sin(LocalLat) * FMath::Sin(LatOut) + FMath::Cos(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);

    return FVector(SX, SY, SZ);
}

FName UDateTimeSystemComponent::GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    if (DateStruct.Month < YearBook.Num())
    {
        return YearBook[DateStruct.Month]->MonthName;
    }

    return FName();
}

void UDateTimeSystemComponent::SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation)
{
    InternalDate = DateStruct;

    // NOTE: This may make DTS change slightly after a load from save... So we can skip this
    if (!SkipInitialisation)
    {
        // Sanitise
        SanitiseDateTime(InternalDate);

        // Reinit
        InternalInitialise();

        InternalTick(0);
    }
}

FDateTimeSystemStruct UDateTimeSystemComponent::GetUTCDateTime()
{
    return InternalDate;
}

void UDateTimeSystemComponent::AdvancedToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    // Technically, we want to compute the delta of Internal to DateStruct, then add it
    auto Delta = DateStruct - InternalDate;
    AddDateStruct(Delta);
}

bool UDateTimeSystemComponent::AdvancedToClockTime(int Hour, int Minute, int Second, bool Safety)
{
    // Struct to adding
    FDateTimeSystemStruct Local{};

    // Step one. Compute the second value of this
    auto ClockTimeInSeconds = (Hour * 60 + Minute) * 60 + Second;
    auto TimeToTravel = ClockTimeInSeconds - InternalDate.Seconds;

    // Step two. Have we already missed this clock time?
    // If we have, we'll get it tomorrow. Unless, we are really close to the wanted time.
    // Set Safety to False to override
    if (TimeToTravel > 0)
    {
        Local.Seconds = TimeToTravel;
        AddDateStruct(Local);
        return true;
    }
    else
    {
        Local.Seconds = TimeToTravel + LengthOfDay;

        // Are we trying to go backwards by less than 30 minutes
        if (Safety && FMath::Abs(TimeToTravel) < 1800)
        {
            // Okay, we want to warn the caller. It's possible that the adjustment
            // is valid, but they need to call with Safety off
            return false;
        }

        AddDateStruct(Local);
        return true;
    }
}

void UDateTimeSystemComponent::AddDateStruct(FDateTimeSystemStruct &DateStruct)
{
    // DayIndex must be maintained
    auto DeltaDayIndex = DateStruct.Day;

    // Compute the advancement of months
    for (int32 i = 0; i < DateStruct.Month; ++i)
    {
        auto CurrentMonthDays = GetDaysInMonth((InternalDate.Month + i) % GetMonthsInYear(InternalDate.Year));
        auto MonthDays = GetDaysInMonth((InternalDate.Month + i + 1) % GetMonthsInYear(InternalDate.Year));

        auto ShortSkip = (InternalDate.Day + 1) - MonthDays;

        if (ShortSkip > 0)
        {
            CurrentMonthDays -= ShortSkip;
        }

        DeltaDayIndex += CurrentMonthDays;
    }

    // Compute the advancement of months
    for (int32 i = 0; i < DateStruct.Year; ++i)
    {
        auto CurrentYearDays = GetLengthOfCalendarYear(InternalDate.Year + i);
        auto CurrentYearLeap = DoesYearLeap(InternalDate.Year + i);
        auto NextYearLeap = DoesYearLeap(InternalDate.Year + i + 1);

        // Hardcoded FebLeap hack
        if (InternalDate.Month > 1 && CurrentYearLeap)
        {
            // GetLengthOfCalendar will be +1 for the day that's already been counted
            --CurrentYearDays;
        }
        else if (InternalDate.Month > 1 && NextYearLeap)
        {
            // Current year will report one short
            ++CurrentYearDays;
        }

        DeltaDayIndex += CurrentYearDays;
    }

    // DateStruct.
    InternalDate += DateStruct;
    InternalDate.DayIndex += DeltaDayIndex;

    // Okay, now we do something a little different
    // DayOfWeek is not updated and it's value is ignored in incoming DateStruct
    InternalDate.DayOfWeek = (InternalDate.DayOfWeek + DeltaDayIndex) % DaysInWeek;

    // Sanitise
    SanitiseDateTime(InternalDate);

    // Reinit
    InternalInitialise();

    InternalTick(0);
}

float UDateTimeSystemComponent::GetFractionalDay(FDateTimeSystemStruct &DateStruct)
{
    float FracDay = (DateStruct.Seconds) * InvLengthOfDay;
    return FracDay;
}

float UDateTimeSystemComponent::GetFractionalMonth(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetFractionalMonth"), STAT_ACIGetFractionalMonth, STATGROUP_ACIDateTimeSys);

    // Yearbook is required for this
    if (DateStruct.Month < YearBook.Num())
    {
        check(YearBook[DateStruct.Month]->NumberOfDays > 0);

        auto FracDay = GetFractionalDay(DateStruct);
        auto LeapMonth = InternalDoesLeap(DateStruct.Year) && YearBook[DateStruct.Month]->AffectedByLeap;
        auto FracMonth = (DateStruct.Day + FracDay) / (YearBook[DateStruct.Month]->NumberOfDays + LeapMonth);

        return FracMonth;
    }

    return 0.0f;
}

float UDateTimeSystemComponent::GetFractionalOrbitalYear(FDateTimeSystemStruct &DateStruct)
{
    float FracDay = InternalDate.SolarDays - 1 + GetFractionalDay(DateStruct);
    float FracYear = FracDay / DaysInOrbitalYear;
    return FMath::Frac(FracYear);
}

float UDateTimeSystemComponent::GetFractionalCalendarYear(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetFractionalCalendarYear"), STAT_ACIGetFractionalCalendarYear,
                                STATGROUP_ACIDateTimeSys);

    // We don't know how long a year is without Yearbook
    if (LengthOfCalendarYearInDays > 0 && DateStruct.Month < YearBook.Num())
    {
        // Okay
        int CumulativeDays = DateStruct.Day;
        for (int32_t i = 0; i < DateStruct.Month; ++i)
        {
            CumulativeDays += YearBook[i]->NumberOfDays;
        }

        float FracDay = GetFractionalDay(DateStruct);
        float FracYear = (CumulativeDays + FracDay) / GetLengthOfCalendarYear(DateStruct.Year);
        return FracYear;
    }

    // Use Orbital
    return GetFractionalOrbitalYear(DateStruct);
}

int UDateTimeSystemComponent::GetLengthOfCalendarYear(int Year)
{
    return LengthOfCalendarYearInDays + InternalDoesLeap(Year);
}

double UDateTimeSystemComponent::GetJulianDay(FDateTimeSystemStruct &DateStruct)
{
    auto JulianSolarDays = 4716 * DaysInOrbitalYear + DateStruct.SolarDays;
    auto JulianPartialDays = (InternalDate.StoredSolarSeconds - LengthOfDay) * InvLengthOfDay;

    return JulianSolarDays + JulianPartialDays;
}

void UDateTimeSystemComponent::DateTimeSetup()
{
    OverridedDatesSetDate = false;
    UseDayIndexForOverride = false;
    DaysInWeek = 7;
    TicksPerSecond = 4;
    TimeScale = 300;
    DaysInOrbitalYear = 365.25;
    LengthOfDay = 60 * 60 * 24;
    // PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    RegisterAllComponentTickFunctions(true);
}

void UDateTimeSystemComponent::Invalidate(
    EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
{
    if (Type >= EDateTimeSystemInvalidationTypes::Year)
    {
        CachedDoesLeap.Valid = false;
    }

    if (Type >= EDateTimeSystemInvalidationTypes::Month)
    {
    }

    // Not cleared on frame-to-frame invalidation
    if (Type >= EDateTimeSystemInvalidationTypes::Day)
    {
    }

    // Clear sun vector helpers
    CachedSolarDeclinationAngle.Valid = false;
    CachedSolarFractionalYear.Valid = false;

    // Clear all sun vectors
    CachedSunVectors.Empty();
}

float UDateTimeSystemComponent::GetDeclinationAngle(FDateTimeSystemStruct &DateStruct)
{
    auto FracDay = DateStruct.Day + DateStruct.Seconds * InvLengthOfDay;

    FracDay -= SolsticeOffsetInDays;

    auto YearFrac = FracDay / DaysInOrbitalYear;

    return 0.4091052 * FMath::Cos(YearFrac * TWO_PI);
    // return 23.44 * FMath::Cos(YearFrac * TWO_PI);
}

bool UDateTimeSystemComponent::HandleDayRollover(FDateTimeSystemStruct &DateStruct)
{
    // Check how many seconds we have
    if (DateStruct.Seconds >= LengthOfDay * 2 || DateStruct.Seconds < -LengthOfDay)
    {
        // Switch to a mode that enables us to handle multiple days.
        // This is not merged as it needs testing
        auto NumberOfDays = FMath::TruncToInt(FMath::Floor(DateStruct.Seconds / LengthOfDay));

        DateStruct.Seconds -= NumberOfDays * LengthOfDay;
        DateStruct.DayOfWeek = (DateStruct.DayOfWeek + NumberOfDays) % DaysInWeek;
        DateStruct.Day += NumberOfDays;
        DateStruct.DayIndex += NumberOfDays;

        return true;
    }
    else if (DateStruct.Seconds > LengthOfDay)
    {
        DateStruct.Seconds -= LengthOfDay;
        DateStruct.DayOfWeek = (DateStruct.DayOfWeek + 1) % DaysInWeek;
        ++DateStruct.Day;
        ++DateStruct.DayIndex;

        return true;
    }
    else if (DateStruct.Seconds < 0)
    {
        DateStruct.Seconds += LengthOfDay;
        DateStruct.DayOfWeek = (DateStruct.DayOfWeek - 1) % DaysInWeek;
        --DateStruct.Day;
        --DateStruct.DayIndex;

        return true;
    }

    return false;
}

bool UDateTimeSystemComponent::HandleMonthRollover(FDateTimeSystemStruct &DateStruct)
{
    auto DaysInMonth = GetDaysInMonth(DateStruct.Month);

    if (DateStruct.Day > DaysInMonth)
    {
        DateStruct.Day -= DaysInMonth;
        ++DateStruct.Month;

        return true;
    }
    else if (DateStruct.Day < 0)
    {
        DateStruct.Day += DaysInMonth;
        --DateStruct.Month;

        return true;
    }

    return false;
}

bool UDateTimeSystemComponent::HandleYearRollover(FDateTimeSystemStruct &DateStruct)
{
    auto MonthsInYear = GetMonthsInYear(DateStruct.Year);
    if (DateStruct.Month > MonthsInYear)
    {
        DateStruct.Month -= MonthsInYear;
        ++DateStruct.Year;

        return true;
    }
    else if (DateStruct.Month < 0)
    {
        DateStruct.Month += MonthsInYear;
        --DateStruct.Year;

        return true;
    }

    return false;
}

int UDateTimeSystemComponent::GetDaysInCurrentMonth()
{
    return GetDaysInMonth(InternalDate.Month);
}

int UDateTimeSystemComponent::GetDaysInMonth(int MonthIndex)
{
    if (MonthIndex < YearBook.Num())
    {
        return YearBook[MonthIndex]->NumberOfDays;
    }
    return 31;
}

int UDateTimeSystemComponent::GetMonthsInYear(int YearIndex)
{
    if (YearBook.Num() > 0)
    {
        return YearBook.Num();
    }
    return 12;
}

bool UDateTimeSystemComponent::SanitiseDateTime(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SanitiseDateTime"), STAT_ACISanitiseDateTime, STATGROUP_ACIDateTimeSys);

    auto DidRolloverDay = HandleDayRollover(DateStruct);
    if (DidRolloverDay)
    {
        // Loop externally to the month function
        // It's cleaner
        auto DidRolloverMonth = false;
        {

            auto ContinueLoop = false;
            do
            {
                ContinueLoop = HandleMonthRollover(DateStruct);
                DidRolloverMonth = ContinueLoop || DidRolloverMonth;
            }
            while (ContinueLoop);
        }

        // Rollover Month and Year
        // auto DidRolloverMonth = HandleMonthRollover(DateStruct);
        if (DidRolloverMonth)
        {
            {
                auto ContinueLoop = false;
                do
                {
                    ContinueLoop = HandleYearRollover(DateStruct);
                    // auto DidRolloverYear = ContinueLoop;
                }
                while (ContinueLoop);
            }
        }

        // Now, we want to fire the datechange
        return true;
    }

    return false;
}

bool UDateTimeSystemComponent::SanitiseSolarDateTime(FDateTimeSystemStruct &DateStruct)
{
    // Check how many seconds we have solar wise
    if (DateStruct.StoredSolarSeconds > LengthOfDay)
    {
        DateStruct.StoredSolarSeconds -= LengthOfDay;
        ++DateStruct.SolarDays;

        return true;
    }

    return false;
}

float UDateTimeSystemComponent::GetSolarFractionalDay()
{
    float FracDay = (InternalDate.StoredSolarSeconds - LengthOfDay * 0.5) * InvLengthOfDay;
    return FracDay;
}

float UDateTimeSystemComponent::GetSolarFractionalYear()
{
    if (CachedSolarFractionalYear.Valid)
    {
        return CachedSolarFractionalYear.Value;
    }

    float FracDays =
        InternalDate.SolarDays - 1 + ((InternalDate.StoredSolarSeconds - LengthOfDay * 0.5) * InvLengthOfDay);
    float YearInRadians = (TWO_PI / DaysInOrbitalYear) * (FracDays);

    CachedSolarFractionalYear.Value = YearInRadians;
    CachedSolarFractionalYear.Valid = true;

    return YearInRadians;
}

float UDateTimeSystemComponent::SolarDeclinationAngle(float YearInRadians)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SolarDeclinationAngle"), STAT_ACISolarDeclinationAngle, STATGROUP_ACIDateTimeSys);

    if (CachedSolarDeclinationAngle.Valid)
    {
        return CachedSolarDeclinationAngle.Value;
    }

    float A1 = 0.006918 - 0.399912 * FMath::Cos(YearInRadians) + 0.070257 * FMath::Sin(YearInRadians) -
               0.006758 * FMath::Cos(2 * YearInRadians) + 0.000907 * FMath::Sin(2 * YearInRadians) -
               0.002697 * FMath::Cos(3 * YearInRadians) + 0.00148 * FMath::Sin(3 * YearInRadians);

    CachedSolarDeclinationAngle.Valid = true;
    CachedSolarDeclinationAngle.Value = A1;

    return A1;
}

bool UDateTimeSystemComponent::InternalDoesLeap(int Year)
{
    // Check Cache
    if (CachedDoesLeap.Valid)
    {
        return bool(CachedDoesLeap.Value);
    }

    CachedDoesLeap.Value = DoesYearLeap(Year);
    CachedDoesLeap.Valid = true;

    return bool(CachedDoesLeap.Value);
}

FRotator UDateTimeSystemComponent::GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                           FVector Location)
{
    auto Lat = GetLatitudeFromLocation(BaseLatitudePercent, Location);
    auto Long = GetLongitudeFromLocation(BaseLatitudePercent, BaseLongitudePercent, Location);
    auto SunInverse = GetSunVector(Lat, Long);
    FVector Flip = -SunInverse;

    return Flip.ToOrientationRotator();
}

FRotator UDateTimeSystemComponent::GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                            FVector Location)
{
    auto Lat = GetLatitudeFromLocation(BaseLatitudePercent, Location);
    auto Long = GetLongitudeFromLocation(BaseLatitudePercent, BaseLongitudePercent, Location);
    auto MoonInverse = GetMoonVector(Lat, Long);
    FVector Flip = -MoonInverse;

    return Flip.ToOrientationRotator();
}

void UDateTimeSystemComponent::InternalTick(float DeltaTime)
{
    // Invalidate Caches
    Invalidate(EDateTimeSystemInvalidationTypes::Frame);

    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("InternalTick"), STAT_ACIInternalTick, STATGROUP_ACIDateTimeSys);

    // Increment Time
    InternalDate.Seconds += DeltaTime * TimeScale;
    InternalDate.StoredSolarSeconds += DeltaTime * TimeScale;
    auto DidRoll = SanitiseDateTime(InternalDate);
    SanitiseSolarDateTime(InternalDate);

    if (DidRoll)
    {
        // Invalidate Daily Caches
        // CachedAnalyticalMonthlyHighTemp.Valid = false;
        // CachedAnalyticalMonthlyLowTemp.Valid = false;
        Invalidate(EDateTimeSystemInvalidationTypes::Day);

        // Check Override
        auto Row = GetDateOverride(&InternalDate);
        if (Row)
        {
            auto asPtr = *Row;

            // We have an override.
            if (OverridedDatesSetDate)
            {
                InternalDate.SetFromRow(asPtr);
                if (DateOverrideCallback.IsBound())
                {
                    DateOverrideCallback.Broadcast(InternalDate, asPtr->CallbackAttributes);
                }
            }
            else
            {
                auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);
                if (DateOverrideCallback.IsBound())
                {
                    DateOverrideCallback.Broadcast(LV, asPtr->CallbackAttributes);
                }
            }
        }

        // Broadcast that the date has changed
        if (DateChangeCallback.IsBound())
        {
            DateChangeCallback.Broadcast(InternalDate);
        }
    }

    if (TimeUpdate.IsBound())
    {
        TimeUpdate.Broadcast(InternalDate);
    }
}

void UDateTimeSystemComponent::InternalBegin()
{
    InvLengthOfDay = 1 / LengthOfDay;
    InvPlanetRadius = 1 / (PlanetRadius * 1000);
    PercentLatitude = FMath::DegreesToRadians(ReferenceLatitude) * INV_PI * 2;
    PercentLongitude = FMath::DegreesToRadians(ReferenceLongitude) * INV_PI;

    InternalDoesLeap(InternalDate.Year);

    // Let's go
    if (YearBookTable)
    {
        YearBookTable->GetAllRows<FDateTimeSystemYearbookRow>(FString("Yearbook Rows"), YearBook);

        LengthOfCalendarYearInDays = 0;
        for (auto val : YearBook)
        {
            LengthOfCalendarYearInDays += val->NumberOfDays;
        }
    }

    if (DateOverridesTable)
    {
        DateOverridesTable->GetAllRows<FDateTimeSystemDateOverrideRow>(FString("Yearbook Rows"), DOTemps);

        for (auto val : DOTemps)
        {
            if (UseDayIndexForOverride)
            {
                DateOverrides.Add(val->DayIndex, val);
            }
            else
            {
                DateOverrides.Add(GetHashForDate(val), val);
            }
        }
    }

    InternalInitialise();
}

void UDateTimeSystemComponent::InternalInitialise()
{
    double Val = InternalDate.Year * DaysInOrbitalYear;
    double Days = GetFractionalCalendarYear(InternalDate) * DaysInOrbitalYear;
    InternalDate.SolarDays = FMath::TruncToInt(Val) + FMath::TruncToInt(Days);
    InternalDate.StoredSolarSeconds = (FMath::Fractional(Val) + FMath::Fractional(Days)) * LengthOfDay;
}

FVector UDateTimeSystemComponent::AlignWorldLocationInternalCoordinates(FVector WorldLocation,
                                                                        FVector NorthingDirection)
{
    // Internally, we use X northing, Y easting, and Z surface normal
    auto InverseRotation = NorthingDirection.ToOrientationRotator().GetInverse();

    auto RetVal = FRotationMatrix(InverseRotation).TransformVector(WorldLocation);

    return FVector(RetVal);
}

float UDateTimeSystemComponent::SolarTimeCorrection(float YearInRadians)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SolarTimeCorrection"), STAT_ACISolarTimeCorrection, STATGROUP_ACIDateTimeSys);

    if (CachedSolarTimeCorrection.Valid)
    {
        return CachedSolarTimeCorrection.Value;
    }

    float A1 = 0.000075 + 0.001868 * FMath::Cos(YearInRadians) - 0.032077 * FMath::Sin(YearInRadians) -
               0.014615 * FMath::Cos(YearInRadians * 2) - 0.040849 * FMath::Sin(YearInRadians * 2);
    float EQTime = 229.18f * A1;

    CachedSolarTimeCorrection.Valid = true;
    CachedSolarTimeCorrection.Value = EQTime;

    return EQTime;
}

uint32 UDateTimeSystemComponent::GetHashForDate(FDateTimeSystemStruct *DateStruct)
{
    auto DayHash = GetTypeHash(DateStruct->Day);
    auto MonthHash = GetTypeHash(DateStruct->Month);
    auto YearHash = GetTypeHash(DateStruct->Year);
    auto Hash = HashCombine(YearHash, MonthHash);
    return HashCombine(Hash, DayHash);
}

uint32 UDateTimeSystemComponent::GetHashForDate(FDateTimeSystemDateOverrideRow *DateStruct)
{
    auto DayHash = GetTypeHash(DateStruct->Day);
    auto MonthHash = GetTypeHash(DateStruct->Month);
    auto YearHash = GetTypeHash(DateStruct->Year);
    auto Hash = HashCombine(YearHash, MonthHash);
    return HashCombine(Hash, DayHash);
}

FDateTimeSystemDateOverrideRow **UDateTimeSystemComponent::GetDateOverride(FDateTimeSystemStruct *DateStruct)
{
    if (UseDayIndexForOverride)
    {
        return DateOverrides.Find(InternalDate.DayIndex);
    }
    else
    {
        return DateOverrides.Find(GetHashForDate(DateStruct));
    }
}
