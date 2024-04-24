// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "DateTimeCommonCore.h"

UDateTimeSystemCore::UDateTimeSystemCore()
    : LengthOfDay(0)
    , InvLengthOfDay(0)
    , CurrentTickIndex(0)
    , PercentLatitude(0)
    , PercentLongitude(0)
    , PlanetRadius(0)
    , InvPlanetRadius(0)
    , UseDayIndexForOverride(false)
    , OverridedDatesSetDate(false)
    , DaysInOrbitalYear(0)
    , DaysInWeek(0)
    , LengthOfCalendarYearInDays(0)
    , CachedSolarFractionalYear()
    , CachedSolarDeclinationAngle()
    , CachedLunarGeocentricDeclinationRightAscSidereal()
    , CachedSolarTimeCorrection()
    , CachedSolarDaysOfYear()
    , CachedDoesLeap()
    , InternalDate()
{
}

UDateTimeSystemCore::UDateTimeSystemCore(UDateTimeSystemCore &Other)
    : LengthOfDay(0)
    , InvLengthOfDay(0)
    , CurrentTickIndex(0)
    , PercentLatitude(0)
    , PercentLongitude(0)
    , PlanetRadius(0)
    , InvPlanetRadius(0)
    , UseDayIndexForOverride(false)
    , OverridedDatesSetDate(false)
    , DaysInOrbitalYear(0)
    , DaysInWeek(0)
    , LengthOfCalendarYearInDays(0)
    , CachedSolarFractionalYear()
    , CachedSolarDeclinationAngle()
    , CachedLunarGeocentricDeclinationRightAscSidereal()
    , CachedSolarTimeCorrection()
    , CachedSolarDaysOfYear()
    , CachedDoesLeap()
    , InternalDate()
{
}

UDateTimeSystemCore::UDateTimeSystemCore(const FObjectInitializer &ObjectInitializer)
    : LengthOfDay(0)
    , InvLengthOfDay(0)
    , CurrentTickIndex(0)
    , PercentLatitude(0)
    , PercentLongitude(0)
    , PlanetRadius(0)
    , InvPlanetRadius(0)
    , UseDayIndexForOverride(false)
    , OverridedDatesSetDate(false)
    , DaysInOrbitalYear(0)
    , DaysInWeek(0)
    , LengthOfCalendarYearInDays(0)
    , CachedSolarFractionalYear()
    , CachedSolarDeclinationAngle()
    , CachedLunarGeocentricDeclinationRightAscSidereal()
    , CachedSolarTimeCorrection()
    , CachedSolarDaysOfYear()
    , CachedDoesLeap()
    , InternalDate()
{
}

void UDateTimeSystemCore::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds;
    DateStruct.Day = InternalDate.Day;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;
}

void UDateTimeSystemCore::GetTodaysDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                          FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    DateStruct = InternalDate;
    DateStruct.Seconds = InternalDate.Seconds + TimezoneInfo.HoursDeltaFromMeridian * 3600;
    DateStruct.Day = InternalDate.Day;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemCore::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Day = InternalDate.Day + 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemCore::GetTomorrowsDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
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

void UDateTimeSystemCore::GetYesterdaysDate(FDateTimeSystemStruct &DateStruct)
{
    DateStruct = InternalDate;
    DateStruct.Day = InternalDate.Day - 1;
    DateStruct.Month = InternalDate.Month;
    DateStruct.Year = InternalDate.Year;

    SanitiseDateTime(DateStruct);
}

void UDateTimeSystemCore::GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
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

float UDateTimeSystemCore::GetLatitudeFromLocation_Implementation(float BaseLatitudePercent, FVector Location)
{
    auto LatLoc = Location.X * 0.01 * InvPlanetRadius * INV_PI * 2; // Divide distance walked by Pi/2
    LatLoc += BaseLatitudePercent;
    const auto ScaledX = LatLoc - 1; // Shift range to from -1 to 1, to -2 to 0
    const auto Modded =
        DateTimeHelpers::HelperMod(ScaledX, 4) - 2; // Take the modulus, which now goes from -2 to 2 when x is -3 to 1
    const auto Absolute = FMath::Abs(Modded) - 1;   // Invert the negatives and give use a triangle wave

    return HALF_PI * Absolute;
}

float UDateTimeSystemCore::GetLongitudeFromLocation_Implementation(float BaseLatitudePercent,
                                                                   float BaseLongitudePercent, FVector Location)
{
    // auto
    auto LatLoc = Location.X * 0.01 * InvPlanetRadius * INV_PI * 2; // Divide distance walked by Pi/2
    auto LongLoc = Location.Y * 0.01 * InvPlanetRadius * INV_PI;

    LatLoc += BaseLatitudePercent;
    LongLoc += BaseLongitudePercent;

    const auto Flipped = DateTimeHelpers::HelperMod((LatLoc - 1) * 0.5, 2) < 1;
    if (Flipped)
    {
        LongLoc += 1; // Flip into the other hemisphere
    }

    const auto ScaledX = LongLoc;
    const auto Modded = DateTimeHelpers::HelperMod(ScaledX, 2);
    // auto Absolute = FMath::Abs(Modded) - 1; // Radians

    // FMath::Wrap(ScaledX, 0.0, 4.0);

    return PI * Modded;
}

bool UDateTimeSystemCore::DoesYearLeap_Implementation(int Year)
{
    if (Year % 4 == 0)
    {
        if (Year % 100 != 0)
        {
            if (Year % 400 == 0)
            {
                return true;
            }

            return false;
        }

        return true;
    }

    return false;
}

FRotator UDateTimeSystemCore::GetSunRotationForLocation_Implementation(FVector Location)
{
    return GetLocalisedSunRotation(PercentLatitude, PercentLongitude, Location);
}

FRotator UDateTimeSystemCore::GetSunRotationForLatLong_Implementation(double Latitude, double Longitude)
{
    const auto LocalisedPercentLatitude = FMath::DegreesToRadians(Latitude) * INV_PI * 2;
    const auto LocalisedPercentLongitude = FMath::DegreesToRadians(Longitude) * INV_PI;

    return GetLocalisedSunRotation(LocalisedPercentLatitude, LocalisedPercentLongitude, FVector::ZeroVector);
}

FRotator UDateTimeSystemCore::GetSunRotation_Implementation()
{
    return GetSunRotationForLocation(FVector::ZeroVector);
}

FVector UDateTimeSystemCore::GetSunVector_Implementation(float Latitude, float Longitude)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetSunVector_Implementation"), STAT_ACIGetSunVector, STATGROUP_ACIDateTimeCommon);

    // Check Cache. We may compute this a few times per frame
    const auto HashType = HashCombine(GetTypeHash(Latitude), GetTypeHash(Longitude));
    const auto Cache = CachedSunVectors.Find(HashType);
    if (Cache)
    {
        return *Cache;
    }

    // https://gml.noaa.gov/grad/solcalc/solareqns.PDF

    const float LocalLat = Latitude;
    const float LocalLong = Longitude;
    const float YearInRads = GetSolarFractionalYear();
    const float DeclAngle = SolarDeclinationAngle(YearInRads);

    // Equation of Time
    const double EQTime = SolarTimeCorrection(YearInRads);

    // New Method
    const float LatOut = DeclAngle;
    const float LongOut = FMath::DegreesToRadians(
        -15 * (((InternalDate.Seconds - LengthOfDay * 0.5) + EQTime * 60) / 3600));
    const float LongDiff = LongOut - LocalLong;
    const float SX = FMath::Cos(LatOut) * FMath::Sin(LongDiff);
    const float SY =
        FMath::Cos(LocalLat) * FMath::Sin(LatOut) - FMath::Sin(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);
    const float SZ =
        FMath::Sin(LocalLat) * FMath::Sin(LatOut) + FMath::Cos(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);

    const auto SunInverse = FVector(SY, SX, SZ).GetSafeNormal();

    CachedSunVectors.Add(HashType, SunInverse);

    return SunInverse;
}

FRotator UDateTimeSystemCore::GetMoonRotationForLocation_Implementation(FVector Location)
{
    return GetLocalisedMoonRotation(PercentLatitude, PercentLongitude, Location);
}

FRotator UDateTimeSystemCore::GetMoonRotation_Implementation()
{
    return GetMoonRotationForLocation(FVector::ZeroVector);
}

FRotator UDateTimeSystemCore::GetMoonRotationForLatLong_Implementation(double Latitude, double Longitude)
{
    const auto LocalisedPercentLatitude = FMath::DegreesToRadians(Latitude) * INV_PI * 2;
    const auto LocalisedPercentLongitude = FMath::DegreesToRadians(Longitude) * INV_PI;

    return GetLocalisedMoonRotation(LocalisedPercentLatitude, LocalisedPercentLongitude, FVector::ZeroVector);
}

/////
// https://www.nrel.gov/docs/fy10osti/47681.pdf
// https://aa.usno.navy.mil/faq/docs/GAST.php
FVector UDateTimeSystemCore::GetMoonVector_Implementation(float Latitude, float Longitude)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetMoonVector_Implementation"), STAT_ACIGetMoonVector,
                                STATGROUP_ACIDateTimeCommon);

    // Check Cache. We may compute this a few times per frame
    const auto HashType = HashCombine(GetTypeHash(Latitude), GetTypeHash(Longitude));
    const auto Cache = CachedMoonVectors.Find(HashType);
    if (Cache)
    {
        return *Cache;
    }

    const auto SinMoonParallax = PlanetRadius / 385000;

    // Non-LatLong dependant compution
    // We use a faster approximation of sidereal time
    // And Declination and Right Ascension are also simplified
    auto DRaSt = LunarDeclinationRightAscensionSiderealTime();
    const auto MoonDeclination = DRaSt.Get<0>();
    const auto MoonRightAscension = DRaSt.Get<1>();
    const auto ApparentSiderealTime = DRaSt.Get<2>();

    const auto HourAngle = ApparentSiderealTime + Longitude - MoonRightAscension;

    const auto FlatteningTerm = FMath::Atan(0.99664719 * FMath::Tan(Latitude));
    const auto ObserverElevationTerm = FMath::Cos(FlatteningTerm);
    const auto TermY = 0.99664719 * FMath::Sin(FlatteningTerm);

    const auto MoonRightAscParallax =
        FMath::Atan2((-ObserverElevationTerm * SinMoonParallax * FMath::Sin(HourAngle)),
                     (FMath::Cos(MoonDeclination) - ObserverElevationTerm * SinMoonParallax * FMath::Cos(HourAngle)));

    // Local Hour Angle: Hour angle with parallax accounted for
    const auto LHA = HourAngle - MoonRightAscParallax;

    // Topocentric Declination
    const auto DeclPrime =
        FMath::Atan2(((FMath::Sin(MoonDeclination) - TermY * SinMoonParallax) * FMath::Cos(MoonRightAscParallax)),
                     (FMath::Cos(MoonDeclination) - TermY * SinMoonParallax * FMath::Cos(HourAngle)));

    // Topocentric Elevation
    const auto MoonTopoElevationAngle = FMath::Asin(FMath::Sin(Latitude) * FMath::Sin(DeclPrime) +
                                                    FMath::Cos(Latitude) * FMath::Cos(DeclPrime) * FMath::Cos(LHA));

    // Topocentric Azimuth
    const auto MoonTopoAzimuthAngle = PI + FMath::Atan2(FMath::Sin(LHA), (FMath::Cos(LHA) * FMath::Sin(Latitude) -
                                                                          FMath::Tan(DeclPrime) * FMath::Cos(
                                                                              Latitude)));

    // Result. Cache and return
    const auto MoonInverse = FVector(FMath::Cos(MoonTopoAzimuthAngle) * FMath::Cos(MoonTopoElevationAngle),
                                     FMath::Sin(MoonTopoAzimuthAngle) * FMath::Cos(MoonTopoElevationAngle),
                                     FMath::Sin(MoonTopoElevationAngle))
        .GetSafeNormal();

    CachedMoonVectors.Add(HashType, MoonInverse);

    return MoonInverse;
}

FMatrix UDateTimeSystemCore::GetNightSkyRotationMatrixForLocation_Implementation(FVector Location)
{
    return GetNightSkyRotation(PercentLatitude, PercentLongitude, Location);
}

FMatrix UDateTimeSystemCore::GetNightSkyRotationMatrix_Implementation()
{
    return GetNightSkyRotationMatrixForLocation(FVector::ZeroVector);
}

FMatrix UDateTimeSystemCore::GetNightSkyRotationMatrixForLatLong_Implementation(double Latitude, double Longitude)
{
    const auto LocalisedPercentLatitude = FMath::DegreesToRadians(Latitude) * INV_PI * 2;
    const auto LocalisedPercentLongitude = FMath::DegreesToRadians(Longitude) * INV_PI;

    return GetNightSkyRotation(LocalisedPercentLatitude, LocalisedPercentLongitude, FVector::ZeroVector);
}

FMatrix UDateTimeSystemCore::GetNightSkyRotation(double PercLatitude, double PercLongitude, FVector Location)
{
    const auto Latitude = GetLatitudeFromLocation(PercLatitude, Location);
    const auto Longitude = GetLongitudeFromLocation(PercLatitude, PercLongitude, Location);

    // +X points north

    // X rotation is the hour angle
    const auto T = GetSolarYears(InternalDate) * 0.01;
    const auto GMST = 6.697374558 + 879'000.051336906897 * T + 0.000026 * T * T;
    const auto GAST = GMST * 15;

    const auto HourAngle = DateTimeHelpers::HelperMod(GAST + FMath::RadiansToDegrees(Longitude), 360.f);

    return FRotationMatrix(FRotator(FMath::RadiansToDegrees(Latitude), 0, HourAngle));
}

float UDateTimeSystemCore::GetMoonApparentLuminosityScale_Implementation(float NewMoonLuminosity,
    float FullMoonLuminosity)
{
    return GetMoonApparentLuminosityScaleForLocation(FVector::ZeroVector, NewMoonLuminosity, FullMoonLuminosity);
}

float UDateTimeSystemCore::GetMoonApparentLuminosityScaleForLocation_Implementation(FVector Location,
                                                                                    float NewMoonLuminosity, float FullMoonLuminosity)
{
    return GetMoonLuminosityScale(PercentLatitude, PercentLongitude, Location, NewMoonLuminosity, FullMoonLuminosity);
}

FText UDateTimeSystemCore::GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    if (DateStruct.Month < YearBook.Num())
    {
        return YearBook[DateStruct.Month]->MonthName;
    }

    return FText::FromString("Missing Month Name");
}

float UDateTimeSystemCore::GetLengthOfDay()
{
    return LengthOfDay;
}

FMatrix UDateTimeSystemCore::RotateMatrixByNorthing(const FMatrix &RotationMatrix, FVector NorthingDirection)
{
    const auto NorthingRotation = NorthingDirection.ToOrientationRotator();
    return FRotationMatrix(NorthingRotation).GetTransposed() * RotationMatrix;
}

void UDateTimeSystemCore::SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation)
{
    InternalDate = DateStruct;

    // NOTE: This may make DTS change slightly after a load from save... So we can skip this
    if (!SkipInitialisation)
    {
        // Sanitise
        SanitiseDateTime(InternalDate);

        // Reinit
        InternalInitialise();

        InternalTick(0, true);
    }
}

FDateTimeSystemStruct UDateTimeSystemCore::GetUTCDateTime()
{
    return InternalDate;
}

void UDateTimeSystemCore::AdvanceToTime(UPARAM(ref) const FDateTimeSystemStruct &DateStruct)
{
    // Technically, we want to compute the delta of Internal to DateStruct, then add it
    auto Delta = DateStruct - InternalDate;
    AddDateStruct(Delta);
}

bool UDateTimeSystemCore::AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety)
{
    // Struct to adding
    FDateTimeSystemStruct Local{};

    // CLAMP
    const auto InternalHour = Hour % 24;
    const auto InternalMinute = Minute % 60;
    const auto InternalSecond = Second % 60;

    // Step one. Compute the second value of this
    const auto ClockTimeInSeconds = (InternalHour * 60 + InternalMinute) * 60 + InternalSecond;
    const auto TimeToTravel = ClockTimeInSeconds - InternalDate.Seconds;

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

float UDateTimeSystemCore::ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                    UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    return ComputeDeltaBetweenDatesYears(Date1, Date2);
}

float UDateTimeSystemCore::ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                         UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    FDateTimeSystemStruct Delta{};
    auto TupleResult = ComputeDeltaBetweenDatesInternal(Date1, Date2, Delta);

    const auto DeltaYears = TupleResult.Get<0>();

    const auto FractionalYear = GetFractionalCalendarYear(Delta);
    auto FractionalMonth = GetFractionalMonth(Delta);

    return DeltaYears + FractionalYear;
}

float UDateTimeSystemCore::ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                          UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    FDateTimeSystemStruct Delta{};
    auto TupleResult = ComputeDeltaBetweenDatesInternal(Date1, Date2, Delta);

    const auto DeltaMonths = TupleResult.Get<1>();

    const auto FractionalMonth = GetFractionalMonth(Delta);

    return DeltaMonths + FractionalMonth;
}

float UDateTimeSystemCore::ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                        UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    FDateTimeSystemStruct Delta{};
    auto TupleResult = ComputeDeltaBetweenDatesInternal(Date1, Date2, Delta);

    const auto DeltaDays = TupleResult.Get<2>();

    const auto FractionalDay = GetFractionalDay(Delta);

    return DeltaDays + FractionalDay;
}

double UDateTimeSystemCore::ComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                            UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    const double Days = ComputeDeltaBetweenDatesDays(Date1, Date2);

    return Days * LengthOfDay;
}

TTuple<float, float, float> UDateTimeSystemCore::ComputeDeltaBetweenDatesInternal(
    UPARAM(ref) FDateTimeSystemStruct &From, UPARAM(ref) FDateTimeSystemStruct &To, FDateTimeSystemStruct &Result)
{
    // Date2 is reference date?
    Result = To - From;

    SanitiseDateTime(Result);

    // The delta needs conversion. AddDateStruct already does this
    // We'll copy and modify rather than creating a function since we need
    // Different stuff

    auto DeltaDays = FMath::Abs(Result.Day);
    auto DeltaMonths = FMath::Abs(Result.Month);
    auto DeltaYears = FMath::Abs(Result.Year);

    // Compute the advancement of months
    for (int32 i = 0; i < Result.Month; ++i)
    {
        auto CurrentMonthDays = GetDaysInMonth((To.Month + i) % GetMonthsInYear(To.Year));
        const auto MonthDays = GetDaysInMonth((To.Month + i + 1) % GetMonthsInYear(To.Year));

        const auto ShortSkip = (To.Day + 1) - MonthDays;

        if (ShortSkip > 0)
        {
            CurrentMonthDays -= ShortSkip;
        }

        DeltaDays += CurrentMonthDays;
    }

    // Compute the advancement of year
    for (int32 i = 0; i < Result.Year; ++i)
    {
        auto CurrentYearDays = GetLengthOfCalendarYear(InternalDate.Year + i);
        const auto CurrentYearLeap = DoesYearLeap(InternalDate.Year + i);
        const auto NextYearLeap = DoesYearLeap(InternalDate.Year + i + 1);

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

        DeltaDays += CurrentYearDays;
    }

    return TTuple<float, float, float>(DeltaYears, DeltaMonths, DeltaDays);
}

void UDateTimeSystemCore::AddDateStruct(FDateTimeSystemStruct &DateStruct)
{
    // DayIndex must be maintained
    auto DeltaDayIndex = DateStruct.Day;

    // Compute the advancement of months
    for (int32 i = 0; i < DateStruct.Month; ++i)
    {
        auto CurrentMonthDays = GetDaysInMonth((InternalDate.Month + i) % GetMonthsInYear(InternalDate.Year));
        const auto MonthDays = GetDaysInMonth((InternalDate.Month + i + 1) % GetMonthsInYear(InternalDate.Year));

        const auto ShortSkip = (InternalDate.Day + 1) - MonthDays;

        if (ShortSkip > 0)
        {
            CurrentMonthDays -= ShortSkip;
        }

        DeltaDayIndex += CurrentMonthDays;
    }

    // Compute the advancement of year
    for (int32 i = 0; i < DateStruct.Year; ++i)
    {
        auto CurrentYearDays = GetLengthOfCalendarYear(InternalDate.Year + i);
        const auto CurrentYearLeap = DoesYearLeap(InternalDate.Year + i);
        const auto NextYearLeap = DoesYearLeap(InternalDate.Year + i + 1);

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

    InternalTick(0, true);
}

float UDateTimeSystemCore::GetMoonApparentLuminosityScaleForLatLong_Implementation(double Latitude, double Longitude,
    float NewMoonLuminosity, float FullMoonLuminosity)
{
    const auto LocalisedPercentLatitude = FMath::DegreesToRadians(Latitude) * INV_PI * 2;
    const auto LocalisedPercentLongitude = FMath::DegreesToRadians(Longitude) * INV_PI;

    return GetMoonLuminosityScale(LocalisedPercentLatitude, LocalisedPercentLongitude, FVector::ZeroVector, NewMoonLuminosity, FullMoonLuminosity);
}

float UDateTimeSystemCore::GetMoonLuminosityScale(double PercLatitude, double PercLongitude, FVector Location,
                                                  float NewMoonLuminosity, float FullMoonLuminosity)
{
    const auto Latitude = GetLatitudeFromLocation(PercLatitude, Location);
    const auto Longitude = GetLongitudeFromLocation(PercLatitude, PercLongitude, Location);

    // Observer to Sun
    auto SunVec = GetSunVector(Latitude, Longitude);

    // Observer to the moon
    auto MoonVec = GetMoonVector(Latitude, Longitude);

    // We care about the vector from the MoonToEarth (-MoonVec) and MoonToSun
    // Given the moon is only 0.002569 au from earth, MoonToSun is approximately just the sun vector
    auto MoonToEarth = -MoonVec;
    auto MoonToSun = SunVec;

    auto BaseScalar = ((FVector::DotProduct(MoonToEarth, MoonToSun) + 1) * 0.5);

    return BaseScalar * (FullMoonLuminosity - NewMoonLuminosity) + NewMoonLuminosity;
}

float UDateTimeSystemCore::GetFractionalDay(FDateTimeSystemStruct &DateStruct)
{
    const float FracDay = (DateStruct.Seconds) * InvLengthOfDay;
    return FracDay;
}

float UDateTimeSystemCore::GetFractionalMonth(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetFractionalMonth"), STAT_ACIGetFractionalMonth, STATGROUP_ACIDateTimeCommon);

    // Yearbook is required for this
    if (DateStruct.Month < YearBook.Num())
    {
        check(YearBook[DateStruct.Month]->NumberOfDays > 0);

        const auto FracDay = GetFractionalDay(DateStruct);
        const auto LeapMonth = InternalDoesLeap(DateStruct.Year) && YearBook[DateStruct.Month]->AffectedByLeap;
        const auto FracMonth = (DateStruct.Day + FracDay) / (YearBook[DateStruct.Month]->NumberOfDays + LeapMonth);

        return FracMonth;
    }

    return 0.0f;
}

float UDateTimeSystemCore::GetFractionalOrbitalYear(FDateTimeSystemStruct &DateStruct)
{
    const float FracDay = InternalDate.SolarDays - 1 + GetFractionalDay(DateStruct);
    const float FracYear = FracDay / DaysInOrbitalYear;
    return FMath::Frac(FracYear);
}

float UDateTimeSystemCore::GetFractionalCalendarYear(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("GetFractionalCalendarYear"), STAT_ACIGetFractionalCalendarYear,
                                STATGROUP_ACIDateTimeCommon);

    // We don't know how long a year is without Yearbook
    if (LengthOfCalendarYearInDays > 0 && DateStruct.Month < YearBook.Num())
    {
        // Okay
        int CumulativeDays = DateStruct.Day;
        for (int32_t i = 0; i < DateStruct.Month; ++i)
        {
            CumulativeDays += YearBook[i]->NumberOfDays;
        }

        const float FracDay = GetFractionalDay(DateStruct);
        const float FracYear = (CumulativeDays + FracDay) / GetLengthOfCalendarYear(DateStruct.Year);
        return FracYear;
    }

    // Use Orbital
    return GetFractionalOrbitalYear(DateStruct);
}

int UDateTimeSystemCore::GetLengthOfCalendarYear(int Year)
{
    return LengthOfCalendarYearInDays + InternalDoesLeap(Year);
}

double UDateTimeSystemCore::GetJulianDay(FDateTimeSystemStruct &DateStruct)
{
    const auto JulianSolarDays = 4716 * DaysInOrbitalYear + DateStruct.SolarDays;
    const auto JulianPartialDays = (InternalDate.StoredSolarSeconds - LengthOfDay) * InvLengthOfDay;

    return JulianSolarDays + JulianPartialDays - 0.5;
}

double UDateTimeSystemCore::GetSolarYears(FDateTimeSystemStruct &DateStruct)
{
    const auto SolarDays = DateStruct.SolarDays + InternalDate.StoredSolarSeconds * InvLengthOfDay;
    const auto SolarYears = SolarDays / DaysInOrbitalYear;

    return SolarYears;
}

void UDateTimeSystemCore::DateTimeSetup()
{
}

void UDateTimeSystemCore::Invalidate(EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Invalidate"), STAT_ACIInvalidate, STATGROUP_ACIDateTimeCommon);

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
    CachedLunarGeocentricDeclinationRightAscSidereal.Valid = false;

    // Clear all sun vectors
    CachedSunVectors.Empty();
    CachedMoonVectors.Empty();

    if (InvalidationCallback.IsBound())
    {
        InvalidationCallback.Broadcast(Type);
    }
}

void UDateTimeSystemCore::RegisterForNotification(TScriptInterface<IDateTimeNotifyInterface> Interface)
{
    NotifiedEntities.Add(Interface.GetObject());
}

void UDateTimeSystemCore::UnregisterForNotification(TScriptInterface<IDateTimeNotifyInterface> Interface)
{
    NotifiedEntities.Remove(Interface.GetObject());
}

bool UDateTimeSystemCore::HandleDayRollover(FDateTimeSystemStruct &DateStruct)
{
    // Check how many seconds we have
    if (DateStruct.Seconds >= LengthOfDay * 2 || DateStruct.Seconds < -LengthOfDay)
    {
        // Switch to a mode that enables us to handle multiple days.
        // This is not merged into the next else if as it needs testing
        const auto NumberOfDays = FMath::TruncToInt(FMath::Floor(DateStruct.Seconds / LengthOfDay));

        DateStruct.Seconds -= NumberOfDays * LengthOfDay;
        DateStruct.DayOfWeek = (DateStruct.DayOfWeek + NumberOfDays) % DaysInWeek;
        DateStruct.Day += NumberOfDays;
        DateStruct.DayIndex += NumberOfDays;

        return true;
    }
    else if (DateStruct.Seconds >= LengthOfDay)
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

bool UDateTimeSystemCore::HandleMonthRollover(FDateTimeSystemStruct &DateStruct)
{
    const auto SafeMonth = DateTimeHelpers::IntHelperMod(DateStruct.Month, GetMonthsInYear(DateStruct.Year));
    const auto DaysInMonth = GetDaysInMonth(SafeMonth);

    if (DateStruct.Day >= DaysInMonth)
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

bool UDateTimeSystemCore::HandleYearRollover(FDateTimeSystemStruct &DateStruct)
{
    const auto MonthsInYear = GetMonthsInYear(DateStruct.Year);
    if (DateStruct.Month >= MonthsInYear)
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

int UDateTimeSystemCore::GetDaysInCurrentMonth()
{
    return GetDaysInMonth(InternalDate.Month);
}

int UDateTimeSystemCore::GetDaysInMonth(int MonthIndex)
{
    if (MonthIndex >= 0 && MonthIndex < YearBook.Num())
    {
        return YearBook[MonthIndex]->NumberOfDays;
    }
    return 31;
}

int UDateTimeSystemCore::GetMonthsInYear(int YearIndex)
{
    if (YearBook.Num() > 0)
    {
        return YearBook.Num();
    }
    return 12;
}

bool UDateTimeSystemCore::SanitiseDateTime(FDateTimeSystemStruct &DateStruct)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SanitiseDateTime"), STAT_ACISanitiseDateTime, STATGROUP_ACIDateTimeCommon);

    const auto DidRolloverDay = HandleDayRollover(DateStruct);

    // Loop externally to the month function
    // It's cleaner as we need yearbook for it
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

    // Year First
    auto DidRolloverYear = false;
    {
        auto ContinueLoop = false;
        do
        {
            ContinueLoop = HandleYearRollover(DateStruct);
            DidRolloverYear = ContinueLoop || DidRolloverYear;
        }
        while (ContinueLoop);
    }

    return DidRolloverDay || DidRolloverMonth || DidRolloverYear;
}

bool UDateTimeSystemCore::SanitiseSolarDateTime(FDateTimeSystemStruct &DateStruct)
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

float UDateTimeSystemCore::GetSolarFractionalDay()
{
    const float FracDay = (InternalDate.StoredSolarSeconds - LengthOfDay * 0.5) * InvLengthOfDay;
    return FracDay;
}

float UDateTimeSystemCore::GetSolarFractionalYear()
{
    if (CachedSolarFractionalYear.Valid)
    {
        return CachedSolarFractionalYear.Value;
    }

    const float FracDays =
        InternalDate.SolarDays - 1 + ((InternalDate.StoredSolarSeconds - LengthOfDay * 0.5) * InvLengthOfDay);
    const float YearInRadians = (TWO_PI / DaysInOrbitalYear) * (FracDays);

    CachedSolarFractionalYear.Value = YearInRadians;
    CachedSolarFractionalYear.Valid = true;

    return YearInRadians;
}

float UDateTimeSystemCore::SolarDeclinationAngle(float YearInRadians)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SolarDeclinationAngle"), STAT_ACISolarDeclinationAngle,
                                STATGROUP_ACIDateTimeCommon);

    if (CachedSolarDeclinationAngle.Valid)
    {
        return CachedSolarDeclinationAngle.Value;
    }

    const float A1 = 0.006918 - 0.399912 * FMath::Cos(YearInRadians) + 0.070257 * FMath::Sin(YearInRadians) -
                     0.006758 * FMath::Cos(2 * YearInRadians) + 0.000907 * FMath::Sin(2 * YearInRadians) -
                     0.002697 * FMath::Cos(3 * YearInRadians) + 0.00148 * FMath::Sin(3 * YearInRadians);

    CachedSolarDeclinationAngle.Valid = true;
    CachedSolarDeclinationAngle.Value = A1;

    return A1;
}

TTuple<double, double, double> UDateTimeSystemCore::LunarDeclinationRightAscensionSiderealTime()
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("LunarDeclinationRightAscensionSiderealTime"), STAT_ACIGetLunarDRaST,
                                STATGROUP_ACIDateTimeCommon);

    if (CachedLunarGeocentricDeclinationRightAscSidereal.Valid)
    {
        // Pack and return
        return TTuple<double, double, double>(CachedLunarGeocentricDeclinationRightAscSidereal.Value1,
                                              CachedLunarGeocentricDeclinationRightAscSidereal.Value2,
                                              CachedLunarGeocentricDeclinationRightAscSidereal.Value3);
    }

    // Shortcut this
    // The US Govt. paper shows 0.00273... which is 1/365.25
    const auto T = GetSolarYears(InternalDate) * 0.01; // JCE
    const auto U = T * 0.01;

    // Geocentric LatLong
    const double GeocentricLongDeg = 218.3164477 + 481'267.88123421 * T - 0.0015786 * T * T +
                                     1.855835023689734077399455498004e-6 * T * T * T -
                                     1.5338834862103874589686167438721e-8 * T * T * T * T;

    const double GeocentricLatRad = 0.089535390624750 * FMath::Sin(1.62839219 + 8433.4662010464 * T) +
                                    0.004886921905444 * FMath::Sin(3.98284135293722 + 16762.15766910478 * T) -
                                    0.004886921905444 * FMath::Sin(5.555383008939 + 104.77473298810291 * T) -
                                    0.002967059728305 * FMath::Sin(3.797836452231 - 7109.2882137217735 * T);

    const auto GeocentricLongRad = FMath::DegreesToRadians(GeocentricLongDeg);

    // Epsilon Term from U
    const auto EpsilonZeroArcSec = 84381.448 - 4680.93 * U - 1.55 * U * U + 1999.25 * U * U * U - 51.38 * U * U * U * U;
    const auto EpsilonZero = FMath::DegreesToRadians(EpsilonZeroArcSec / 3600);

    const auto GMST = 6.697374558 + 879'000.051336906897 * T + 0.000026 * T * T;

    auto MoonDeclination =
        FMath::Asin(FMath::Sin(GeocentricLatRad) * FMath::Cos(EpsilonZero) +
                    FMath::Cos(GeocentricLatRad) * FMath::Sin(EpsilonZero) * FMath::Sin(GeocentricLongRad));

    auto MoonRightAscension = FMath::Atan2(FMath::Sin(GeocentricLongRad) * FMath::Cos(EpsilonZero) -
                                           FMath::Tan(GeocentricLatRad) * FMath::Sin(EpsilonZero),
                                           FMath::Cos(GeocentricLongRad));

    auto GAST = FMath::DegreesToRadians(GMST * 15);

    CachedLunarGeocentricDeclinationRightAscSidereal.Valid = true;
    CachedLunarGeocentricDeclinationRightAscSidereal.Value1 = MoonDeclination;
    CachedLunarGeocentricDeclinationRightAscSidereal.Value2 = MoonRightAscension;
    CachedLunarGeocentricDeclinationRightAscSidereal.Value3 = GAST;

    return TTuple<double, double, double>(MoonDeclination, MoonRightAscension, GAST);
}

bool UDateTimeSystemCore::InternalDoesLeap(int Year)
{
    // Check Cache
    if (CachedDoesLeap.Valid)
    {
        return static_cast<bool>(CachedDoesLeap.Value);
    }

    CachedDoesLeap.Value = DoesYearLeap(Year);
    CachedDoesLeap.Valid = true;

    return static_cast<bool>(CachedDoesLeap.Value);
}

FRotator UDateTimeSystemCore::GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                      FVector Location)
{
    const auto Lat = GetLatitudeFromLocation(BaseLatitudePercent, Location);
    const auto Long = GetLongitudeFromLocation(BaseLatitudePercent, BaseLongitudePercent, Location);
    const auto SunInverse = GetSunVector(Lat, Long);
    const FVector Flip = -SunInverse;

    return Flip.ToOrientationRotator();
}

FRotator UDateTimeSystemCore::GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                       FVector Location)
{
    const auto Lat = GetLatitudeFromLocation(BaseLatitudePercent, Location);
    const auto Long = GetLongitudeFromLocation(BaseLatitudePercent, BaseLongitudePercent, Location);
    const auto MoonInverse = GetMoonVector(Lat, Long);
    const FVector Flip = -MoonInverse;

    return Flip.ToOrientationRotator();
}

void UDateTimeSystemCore::InternalTick(float DeltaTime, bool NonContiguous)
{
    // Invalidate Caches
    Invalidate(EDateTimeSystemInvalidationTypes::Frame);

    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("InternalTick"), STAT_ACIInternalTick, STATGROUP_ACIDateTimeCommon);

    // Increment Time
    InternalDate.Seconds += DeltaTime;
    InternalDate.StoredSolarSeconds += DeltaTime;
    const auto DidRoll = SanitiseDateTime(InternalDate);
    SanitiseSolarDateTime(InternalDate);

    if (DidRoll || NonContiguous)
    {
        // Invalidate Daily Caches
        if (NonContiguous)
        {
            // We actually don't know how far we skipped, so invalidate everything
            Invalidate(EDateTimeSystemInvalidationTypes::Year);
        }
        else
        {
            Invalidate(EDateTimeSystemInvalidationTypes::Day);
        }

        // Check Override
        const auto Row = GetDateOverride(&InternalDate);
        if (Row)
        {
            const auto asPtr = *Row;

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
                const auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);
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

        // Notify Entities
        for (const auto &NotifiableEntity : NotifiedEntities)
        {
            const auto EntityObject = NotifiableEntity.GetObject();
            if (IsValid(EntityObject))
            {
                const auto EntityAsInterface = Cast<IDateTimeNotifyInterface>(EntityObject);
                if (EntityAsInterface)
                {
                    EntityAsInterface->DateNotify(InternalDate);
                }
            }
        }
    }

    if (TimeUpdate.IsBound())
    {
        TimeUpdate.Broadcast(InternalDate);
    }

    if (CleanTimeUpdate.IsBound())
    {
        CleanTimeUpdate.Broadcast();
    }
}

void UDateTimeSystemCore::InternalBegin(FDateTimeCommonCoreInitializer &CoreInitializer)
{
    LengthOfDay = CoreInitializer.LengthOfDay;
    PlanetRadius = CoreInitializer.PlanetRadius;
    UseDayIndexForOverride = CoreInitializer.UseDayIndexForOverride;
    OverridedDatesSetDate = CoreInitializer.OverridedDatesSetDate;
    DaysInOrbitalYear = CoreInitializer.DaysInOrbitalYear;
    DaysInWeek = CoreInitializer.DaysInWeek;

    InvLengthOfDay = 1 / LengthOfDay;
    InvPlanetRadius = 1 / (PlanetRadius * 1000);

    const auto ReferenceLatitude = CoreInitializer.ReferenceLatitude;
    const auto ReferenceLongitude = CoreInitializer.ReferenceLongitude;

    PercentLatitude = FMath::DegreesToRadians(ReferenceLatitude) * INV_PI * 2;
    PercentLongitude = FMath::DegreesToRadians(ReferenceLongitude) * INV_PI;

    InternalDate = CoreInitializer.StartDate;

    // Let's go
    if (CoreInitializer.YearbookTable)
    {
        TArray<FDateTimeSystemYearbookRow *> LocalYearbook;
        CoreInitializer.YearbookTable->GetAllRows<FDateTimeSystemYearbookRow>(FString("Yearbook Rows"), LocalYearbook);

        LengthOfCalendarYearInDays = 0;
        for (const auto val : LocalYearbook)
        {
            LengthOfCalendarYearInDays += val->NumberOfDays;

            YearBook.Add(DateTimeRowHelpers::CreateYearbookRowFromTableRow(val));
        }
    }

    if (CoreInitializer.DateOverridesTable)
    {
        TArray<FDateTimeSystemDateOverrideRow *> LocalDOTemps;
        CoreInitializer.DateOverridesTable->GetAllRows<FDateTimeSystemDateOverrideRow>(FString("Yearbook Rows"),
            LocalDOTemps);

        for (const auto val : LocalDOTemps)
        {
            auto DateOverridePtr = DateTimeRowHelpers::CreateOverrideItemFromTableRow(val);
            if (UseDayIndexForOverride)
            {
                DateOverrides.Add(DateOverridePtr->DayIndex, DateOverridePtr);
            }
            else
            {
                DateOverrides.Add(GetHashForDate(DateOverridePtr), DateOverridePtr);
            }
        }
    }

    InternalInitialise();
}

void UDateTimeSystemCore::InternalInitialise()
{
    // Sanity
    if (0 == YearBook.Num())
    {
        UE_LOG(LogDateTimeSystem, Error, TEXT("Yearbook is not valid"));
        return;
    }

    // Sanitise
    SanitiseDateTime(InternalDate);
    // Cache Leap
    InternalDoesLeap(InternalDate.Year);

    const double Val = InternalDate.Year * DaysInOrbitalYear;
    const double Days = GetFractionalCalendarYear(InternalDate) * DaysInOrbitalYear;
    InternalDate.SolarDays = FMath::TruncToInt(Val) + FMath::TruncToInt(Days);
    InternalDate.StoredSolarSeconds = (FMath::Fractional(Val) + FMath::Fractional(Days)) * LengthOfDay;
}

FVector UDateTimeSystemCore::AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection)
{
    // Internally, we use X northing, Y easting, and Z surface normal
    const auto InverseRotation = NorthingDirection.ToOrientationRotator().GetInverse();

    const auto RetVal = FRotationMatrix(InverseRotation).TransformVector(WorldLocation);

    return FVector(RetVal);
}

FVector UDateTimeSystemCore::RotateLocationByNorthing(FVector Location, FVector NorthingDirection)
{
    const auto NorthingRotation = NorthingDirection.ToOrientationRotator();
    const auto RetVal = FRotationMatrix(NorthingRotation).TransformVector(Location);

    return FVector(RetVal);
}

FRotator UDateTimeSystemCore::RotateRotationByNorthing(FRotator Rotation, FVector NorthingDirection)
{
    const auto NorthingRotation = NorthingDirection.ToOrientationRotator();
    return NorthingRotation + Rotation;
}

float UDateTimeSystemCore::SolarTimeCorrection(float YearInRadians)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SolarTimeCorrection"), STAT_ACISolarTimeCorrection, STATGROUP_ACIDateTimeCommon);

    if (CachedSolarTimeCorrection.Valid)
    {
        return CachedSolarTimeCorrection.Value;
    }

    const float A1 = 0.000075 + 0.001868 * FMath::Cos(YearInRadians) - 0.032077 * FMath::Sin(YearInRadians) -
                     0.014615 * FMath::Cos(YearInRadians * 2) - 0.040849 * FMath::Sin(YearInRadians * 2);
    const float EQTime = 229.18f * A1;

    CachedSolarTimeCorrection.Valid = true;
    CachedSolarTimeCorrection.Value = EQTime;

    return EQTime;
}

uint32 UDateTimeSystemCore::GetHashForDate(FDateTimeSystemStruct *DateStruct)
{
    const auto DayHash = GetTypeHash(DateStruct->Day);
    const auto MonthHash = GetTypeHash(DateStruct->Month);
    const auto YearHash = GetTypeHash(DateStruct->Year);
    const auto Hash = HashCombine(YearHash, MonthHash);
    return HashCombine(Hash, DayHash);
}

uint32 UDateTimeSystemCore::GetHashForDate(UDateTimeSystemDateOverrideItem *DateStruct)
{
    const auto DayHash = GetTypeHash(DateStruct->Day);
    const auto MonthHash = GetTypeHash(DateStruct->Month);
    const auto YearHash = GetTypeHash(DateStruct->Year);
    const auto Hash = HashCombine(YearHash, MonthHash);
    return HashCombine(Hash, DayHash);
}

UDateTimeSystemDateOverrideItem **UDateTimeSystemCore::GetDateOverride(FDateTimeSystemStruct *DateStruct)
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
