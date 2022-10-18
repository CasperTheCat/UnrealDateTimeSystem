// [TEMPLATE_COPYRIGHT]

#include "DateTimeSystemComponent.h"

UDateTimeSystemComponent::UDateTimeSystemComponent()
{
	DateTimeSetup();
}

UDateTimeSystemComponent::UDateTimeSystemComponent(UDateTimeSystemComponent& Other)
{
	DateTimeSetup();
}

UDateTimeSystemComponent::UDateTimeSystemComponent(const FObjectInitializer& ObjectInitializer)
{
	DateTimeSetup();
}

void UDateTimeSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InternalTick(DeltaTime);	
}

void UDateTimeSystemComponent::BeginPlay()
{
	//SetComponentTickInterval(1.0 / TicksPerSecond);
	Super::BeginPlay();

	InternalBegin();
}

void UDateTimeSystemComponent::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct& DateStruct)
{
	DateStruct.Seconds = InternalDate.Seconds;
	DateStruct.Day = InternalDate.Day;
	DateStruct.Month = InternalDate.Month;
	DateStruct.Year = InternalDate.Year;
}

void UDateTimeSystemComponent::GetTodaysDateTZ(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FDateTimeSystemTimezoneStruct& TimezoneInfo)
{
	DateStruct.Seconds = InternalDate.Seconds + TimezoneInfo.HoursDeltaFromMeridian * 3600;
	DateStruct.Day = InternalDate.Day;
	DateStruct.Month = InternalDate.Month;
	DateStruct.Year = InternalDate.Year;

	SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct& DateStruct)
{
	DateStruct.Day = InternalDate.Day + 1;
	DateStruct.Month = InternalDate.Month;
	DateStruct.Year = InternalDate.Year;

	SanitiseDateTime(DateStruct);
}

void UDateTimeSystemComponent::GetTomorrowsDateTZ(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FDateTimeSystemTimezoneStruct& TimezoneInfo)
{
	DateStruct.Seconds = InternalDate.Seconds + LengthOfDay + TimezoneInfo.HoursDeltaFromMeridian * 3600;
	HandleDayRollover(DateStruct);

	DateStruct.Day = InternalDate.Day + 1;
	DateStruct.Month = InternalDate.Month;
	DateStruct.Year = InternalDate.Year;

	SanitiseDateTime(DateStruct);
}

float UDateTimeSystemComponent::GetLatitudeFromLocation_Implementation(FVector Location)
{
	return FMath::DegreesToRadians(ReferenceLatitude);
}

float UDateTimeSystemComponent::GetLongitudeFromLocation_Implementation(FVector Location)
{
	return FMath::DegreesToRadians(ReferenceLongitude);
}

bool UDateTimeSystemComponent::DoesYearLeap_Implementation(int Year)
{
	return false;
}

FRotator UDateTimeSystemComponent::GetSunRotationForLocation_Implementation(FVector Location, FDateTimeSystemTimezoneStruct& TimezoneInfo)
{
	auto Lat = GetLatitudeFromLocation(Location);
	auto Long = GetLongitudeFromLocation(Location);
	auto SunInverse = GetSunVector(Lat, Long);
	FVector Flip = -SunInverse;

	return Flip.ToOrientationRotator();
}

FRotator UDateTimeSystemComponent::GetSunRotation_Implementation(FDateTimeSystemTimezoneStruct TimezoneInfo)
{
	return GetSunRotationForLocation(FVector::ZeroVector, TimezoneInfo);
}

FVector UDateTimeSystemComponent::GetSunVector_Implementation(float Latitude, float Longitude)
{
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
	//float DeclAngle = GetDeclinationAngle(InternalDate); // This is a hack. We can use SolarDecl... for a better one
	float DeclAngle = SolarDeclinationAngle(YearInRads);

	// Correct to meridian solar time
	//float SolarHour = InternalDate.Seconds - TimezoneInfo.HoursDeltaFromMeridian * 3600;
	float EQTime = SolarTimeCorrection(YearInRads);

	// TimeOffset is acting 
	float TimeOffset = EQTime + 4 * LocalLong;// -60 * TimezoneInfo.HoursDeltaFromMeridian;

	// Get the time in minutes, and add the offset, also in minutes
	float CorrectedTime = InternalDate.GetTimeInMinutes() + TimeOffset;
	float SolarHour = (CorrectedTime / 4) - 180;
	float SolarAngle = FMath::DegreesToRadians(SolarHour);

	// New Method
	float LatOut = DeclAngle;
	float LongOut = FMath::DegreesToRadians(-15 * (((InternalDate.Seconds - LengthOfDay * 0.5) + EQTime * 60) / 3600));
	float LongDiff = LongOut - LocalLong;
	float SX = FMath::Cos(LatOut) * FMath::Sin(LongDiff);
	float SY = FMath::Cos(LocalLat) * FMath::Sin(LatOut) - FMath::Sin(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);
	float SZ = FMath::Sin(LocalLat) * FMath::Sin(LatOut) + FMath::Cos(LocalLat) * FMath::Cos(LatOut) * FMath::Cos(LongDiff);

	auto SunInverse = FVector(SY, SX, SZ);

	CachedSunVectors.Add(HashType, SunInverse);

	return SunInverse;






	//// Time for Zenith
	//float SolarZenith = FMath::Sin(LocalLat) * FMath::Sin(DeclAngle) +
	//	FMath::Cos(LocalLat) * FMath::Cos(DeclAngle) * FMath::Cos(SolarAngle);
	//float SolarElevation = FMath::Acos(SolarZenith);

	//float SolarAzimuth = (
	//	(FMath::Sin(DeclAngle) - FMath::Sin(LocalLat) * SolarZenith)
	//	/ FMath::Cos(LocalLat) * FMath::Sin(SolarElevation));
	//float SolarYaw = FMath::RadiansToDegrees(FMath::Acos(SolarAzimuth));


	//return FRotator(-90 - FMath::RadiansToDegrees(SolarElevation), 180 - SolarYaw, 0);
}

float UDateTimeSystemComponent::GetMonthlyHighTemperature(int MonthIndex)
{
	checkNoEntry();
	//if (MonthIndex < YearBook.Num())
	//{
	//	
	//	return YearBook[MonthIndex]->MonthlyHighTemp;
	//}
	return 0.0f;
}

float UDateTimeSystemComponent::GetMonthlyLowTemperature(int MonthIndex)
{
	checkNoEntry();
	//if (MonthIndex < YearBook.Num())
	//{
	//	return YearBook[MonthIndex]->MonthlyLowTemp;
	//}
	return 0.0f;
}

void UDateTimeSystemComponent::DummyAddTick(float Time)
{
	InternalTick(Time);
}

FName UDateTimeSystemComponent::GetNameOfMonth(UPARAM(ref)FDateTimeSystemStruct& DateStruct)
{
	if (DateStruct.Month < YearBook.Num())
	{
		return YearBook[DateStruct.Month]->MonthName;
	}

	return FName();
}

float UDateTimeSystemComponent::DailyLowModulation_Implementation(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh)
{
	return Temperature;
}

float UDateTimeSystemComponent::DailyHighModulation_Implementation(UPARAM(ref)FDateTimeSystemStruct& DateStruct, FGameplayTagContainer& Attributes, float Temperature, float PreviousDayLow, float PreviousDayHigh)
{
	return Temperature;
}

float UDateTimeSystemComponent::ModulateTemperature_Implementation(FVector Location, float Temperature, float SecondsSinceUpdate, float LowTemperature, float HighTemperature, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo)
{
	// Get the sun vector. We want to suppress the temperature when it's below the horizon
	
	//auto Lat = GetLatitudeFromLocation(Location);
	//auto Long = GetLongitudeFromLocation(Location);
	//auto SunVector = GetSunVector(Lat, Long);

	//if (FVector::DotProduct(SunVector, FVector::UpVector) > 0)
	//{
	//	// Sun is above the horizon
	//	// We want to start Lerping to high temperature slowly
	//	return FMath::FInterpTo(Temperature, HighTemperature, SecondsSinceUpdate, 0.04);
	//}
	//else
	//{
	//	return FMath::FInterpTo(Temperature, LowTemperature, SecondsSinceUpdate, 0.04);
	//}

	FDateTimeSystemStruct LocalDateStruct{};
	GetTodaysDateTZ(LocalDateStruct, TimezoneInfo);

	auto FracDay = GetFractionalDay(LocalDateStruct);
	auto Multi = FMath::Sin(PI * FracDay);
	return FMath::Lerp(LowTemperature, HighTemperature, Multi);
}

float UDateTimeSystemComponent::GetCurrentTemperature(FVector Location, float CurrentTemperature, float SecondsSinceUpdate, UPARAM(ref) FDateTimeSystemTimezoneStruct& TimezoneInfo)
{
	// We're disabled...
	checkNoEntry();

	// Okay. Our Current Temp is going to be dumb.
	// Which low are we using? The first or last?

	FDateTimeSystemStruct LocalDateStruct{};
	GetTodaysDateTZ(LocalDateStruct, TimezoneInfo);

	auto FracDay = GetFractionalDay(LocalDateStruct);

	auto HighTemp = GetDailyHigh(LocalDateStruct);
	auto LowTemp = CachedLowTemp.Value;
	if (FracDay > 0.5)
	{
		// Need today's low, rather than the prior
		LowTemp = GetDailyLow(LocalDateStruct);
	}

	auto Output = ModulateTemperature(Location, CurrentTemperature, SecondsSinceUpdate, LowTemp, HighTemp, TimezoneInfo);
	
	return Output;
}

float UDateTimeSystemComponent::GetFractionalDay(FDateTimeSystemStruct& DateStruct)
{
	float FracDay = (DateStruct.Seconds) * InvLengthOfDay;
	return FracDay;
}

float UDateTimeSystemComponent::GetFractionalMonth(FDateTimeSystemStruct& DateStruct)
{
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

float UDateTimeSystemComponent::GetFractionalOrbitalYear(FDateTimeSystemStruct& DateStruct)
{
	float FracDay = InternalDate.DayIndex - 1 + GetFractionalDay(DateStruct);
	float FracYear = FracDay / DaysInOrbitalYear;
	return FMath::Frac(FracYear);
}

float UDateTimeSystemComponent::GetFractionalCalendarYear(FDateTimeSystemStruct& DateStruct)
{
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

float UDateTimeSystemComponent::GetAnalyticalHighForDate(FDateTimeSystemStruct& DateStruct)
{
	checkNoEntry();
	// We have two things to do here. Return the cache, if it's valid
	//if (CachedAnalyticalMonthlyHighTemp.Valid)
	//{
	//	return CachedAnalyticalMonthlyHighTemp.Value;
	//}
	//else
	//{
	//	// We need 
	//	if (DateStruct.Month < YearBook.Num())
	//	{
	//		// Which do we need. We need the fractional month value
	//		auto MonthFrac = GetFractionalMonth(DateStruct);
	//		auto CurrentMonthHigh = YearBook[DateStruct.Month]->MonthlyHighTemp;
	//		auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

	//		if (MonthFrac > 0.5)
	//		{
	//			// Future Month
	//			auto OtherIndex = (DateStruct.Month + 1) % YearBook.Num();
	//			auto OtherValue = YearBook[OtherIndex]->MonthlyHighTemp;

	//			// High is lerp frac
	//			CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(CurrentMonthHigh, OtherValue, BlendFrac);
	//		}
	//		else
	//		{
	//			// Future Month
	//			auto OtherIndex = (YearBook.Num() + (DateStruct.Month - 1)) % YearBook.Num();
	//			auto OtherValue = YearBook[OtherIndex]->MonthlyHighTemp;

	//			// High is lerp frac
	//			CachedAnalyticalMonthlyHighTemp.Value = FMath::Lerp(OtherValue, CurrentMonthHigh, BlendFrac);
	//		}
	//		CachedAnalyticalMonthlyHighTemp.Valid = true;
	//		return CachedAnalyticalMonthlyHighTemp.Value;
	//	}
	//}
	return 0.0f;
}

float UDateTimeSystemComponent::GetAnalyticalLowForDate(FDateTimeSystemStruct& DateStruct)
{
	checkNoEntry();
	// We have two things to do here. Return the cache, if it's valid
	//if (CachedAnalyticalMonthlyLowTemp.Valid)
	//{
	//	return CachedAnalyticalMonthlyLowTemp.Value;
	//}
	//else
	//{
	//	// We need 
	//	if (DateStruct.Month < YearBook.Num())
	//	{
	//		// Which do we need. We need the fractional month value
	//		auto MonthFrac = GetFractionalMonth(DateStruct);
	//		auto CurrentMonthLow = YearBook[DateStruct.Month]->MonthlyLowTemp;
	//		auto BlendFrac = FMath::Abs(MonthFrac - 0.5);

	//		if (MonthFrac > 0.5)
	//		{
	//			// Future Month
	//			auto OtherIndex = (DateStruct.Month + 1) % YearBook.Num();
	//			auto OtherValue = YearBook[OtherIndex]->MonthlyLowTemp;

	//			// High is lerp frac
	//			CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(CurrentMonthLow, OtherValue, BlendFrac);
	//		}
	//		else
	//		{
	//			// Future Month
	//			auto OtherIndex = (YearBook.Num() + (DateStruct.Month - 1)) % YearBook.Num();
	//			auto OtherValue = YearBook[OtherIndex]->MonthlyLowTemp;

	//			// High is lerp frac
	//			CachedAnalyticalMonthlyLowTemp.Value = FMath::Lerp(OtherValue, CurrentMonthLow, BlendFrac);
	//		}
	//		CachedAnalyticalMonthlyLowTemp.Valid = true;
	//		return CachedAnalyticalMonthlyLowTemp.Value;
	//	}
	//}
	return 0.0f;
}

float UDateTimeSystemComponent::GetDailyHigh(FDateTimeSystemStruct& DateStruct)
{
	checkNoEntry();
	return 0;
	//if (CachedHighTemp.Valid)
	//{
	//	return CachedHighTemp.Value;
	//}

	//// Check Override
	//auto Row = GetDateOverride(&DateStruct);
	//if (Row)
	//{
	//	auto asPtr = *Row;
	//	auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);

	//	CachedHighTemp.Value = DailyHighModulation(LV, asPtr->CallbackAttributes, asPtr->HighTemp, LastLowTemp, LastHighTemp);
	//	CachedHighTemp.Valid = true;
	//}
	//else
	//{
	//	// Go ahead and compute the new values
	//	CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(InternalDate);
	//	CachedAnalyticalMonthlyHighTemp.Valid = true;

	//	auto DummyTagContainer = FGameplayTagContainer();

	//	CachedHighTemp.Value = DailyHighModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value, LastLowTemp, LastHighTemp);
	//	CachedHighTemp.Valid = true;
	//}

	//return CachedHighTemp.Value;
}

float UDateTimeSystemComponent::GetDailyLow(FDateTimeSystemStruct& DateStruct)
{
	checkNoEntry();
	// Okay. We need to work out which Low we actually want
	return 0;

	//if (CachedNextLowTemp.Valid)
	//{
	//	return CachedNextLowTemp.Value;
	//}

	//// Check Override
	//auto Row = GetDateOverride(&DateStruct);
	//if (Row)
	//{
	//	auto asPtr = *Row;
	//	auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);

	//	CachedNextLowTemp.Value = DailyLowModulation(LV, asPtr->CallbackAttributes, asPtr->LowTemp, LastLowTemp, LastHighTemp);
	//	CachedNextLowTemp.Valid = true;
	//}
	//else
	//{
	//	// Go ahead and compute the new values
	//	CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(InternalDate);
	//	CachedAnalyticalMonthlyLowTemp.Valid = true;

	//	auto DummyTagContainer = FGameplayTagContainer();

	//	CachedNextLowTemp.Value = DailyLowModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, LastLowTemp, LastHighTemp);
	//	CachedNextLowTemp.Valid = true;
	//}

	//return CachedNextLowTemp.Value;

	//if (CachedLowTemp.Valid)
	//{
	//	return CachedLowTemp.Value;
	//}

	//// Handle rolling the starting temp of day n+1 to the ending of n
	//CachedLowTemp.Value = CachedNextLowTemp.Value;
	//CachedLowTemp.Valid = true;

	//// Check Override
	//auto Row = DateOverrides.Find(InternalDate.DayIndex);
	//if (Row)
	//{
	//	auto LV = FDateTimeSystemStruct::CreateFromRow(Row);

	//	CachedNextLowTemp.Value = DailyLowModulation(LV, Row->CallbackAttributes, Row->LowTemp, CachedLowTemp.Value, CachedHighTemp.Value);
	//	CachedHighTemp.Value = DailyHighModulation(LV, Row->CallbackAttributes, Row->HighTemp, CachedLowTemp.Value, CachedHighTemp.Value);

	//	CachedNextLowTemp.Valid = true;
	//	CachedHighTemp.Valid = true;
	//}
	//else
	//{
	//	// Go ahead and compute the new values
	//	CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(InternalDate);
	//	CachedAnalyticalMonthlyHighTemp.Valid = true;
	//	CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(InternalDate);
	//	CachedAnalyticalMonthlyLowTemp.Valid = true;

	//	auto DummyTagContainer = FGameplayTagContainer();

	//	CachedNextLowTemp.Value = DailyLowModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);
	//	CachedHighTemp.Value = DailyHighModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);

	//	CachedNextLowTemp.Valid = true;
	//	CachedHighTemp.Valid = true;
	//}

	//return CachedHighTemp.Value;
}

int UDateTimeSystemComponent::GetLengthOfCalendarYear(int Year)
{
	return LengthOfCalendarYearInDays + InternalDoesLeap(Year);
}

void UDateTimeSystemComponent::DateTimeSetup()
{
	OverridedDatesSetDate = false;
	UseDayIndexForOverride = false;
	TicksPerSecond = 4;
	TimeScale = 300;
	DaysInOrbitalYear = 365.25;
	LengthOfDay = 60 * 60 * 24;
	//PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	RegisterAllComponentTickFunctions(true);
}

void UDateTimeSystemComponent::Invalidate(EDateTimeSystemInvalidationTypes Type = EDateTimeSystemInvalidationTypes::Frame)
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
		CachedHighTemp.Valid = false;
		CachedLowTemp.Valid = false;
		CachedNextLowTemp.Valid = false;
		CachedAnalyticalMonthlyHighTemp.Valid = false;
		CachedAnalyticalMonthlyLowTemp.Valid = false;
	}

	// Clear sun vector helpers
	CachedSolarDeclinationAngle.Valid = false;
	CachedSolarFractionalYear.Valid = false;

	// Clear all sun vectors
	CachedSunVectors.Empty();
}

float UDateTimeSystemComponent::GetDeclinationAngle(FDateTimeSystemStruct& DateStruct)
{
	auto FracDay = DateStruct.Day + DateStruct.Seconds * InvLengthOfDay;

	FracDay -= SolsticeOffsetInDays;

	auto YearFrac = FracDay / DaysInOrbitalYear;

	return 0.4091052 * FMath::Cos(YearFrac * TWO_PI);
	//return 23.44 * FMath::Cos(YearFrac * TWO_PI);
}

bool UDateTimeSystemComponent::HandleDayRollover(FDateTimeSystemStruct& DateStruct)
{
	// Check how many seconds we have
	if (DateStruct.Seconds > LengthOfDay)
	{
		DateStruct.Seconds -= LengthOfDay;
		++DateStruct.Day;
		++DateStruct.DayIndex;

		return true;
	}
	else if (DateStruct.Seconds < 0)
	{
		DateStruct.Seconds += LengthOfDay;
		--DateStruct.Day;
		--DateStruct.DayIndex;

		return true;
	}

	return false;
}

bool UDateTimeSystemComponent::HandleMonthRollover(FDateTimeSystemStruct& DateStruct)
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

bool UDateTimeSystemComponent::HandleYearRollover(FDateTimeSystemStruct& DateStruct)
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

bool UDateTimeSystemComponent::SanitiseDateTime(FDateTimeSystemStruct& DateStruct)
{
	auto DidRolloverDay = HandleDayRollover(DateStruct);
	if (DidRolloverDay)
	{
		// Rollover Month and Year
		auto DidRolloverMonth = HandleMonthRollover(DateStruct);
		if (DidRolloverMonth)
		{
			auto DidRolloverYear = HandleYearRollover(DateStruct);
		}

		// Now, we want to fire the datechange
		return true;
	}

	return false;
}

float UDateTimeSystemComponent::GetSolarFractionalDay()
{
	float FracDay = (InternalDate.Seconds - LengthOfDay * 0.5) * InvLengthOfDay;
	return FracDay;
}

float UDateTimeSystemComponent::GetSolarFractionalYear()
{
	if (CachedSolarFractionalYear.Valid)
	{
		return CachedSolarFractionalYear.Value;
	}

	float FracDays = InternalDate.DayIndex - 1 + (
		(InternalDate.Seconds - LengthOfDay * 0.5) * InvLengthOfDay
		);
	float YearInRadians = (TWO_PI / DaysInOrbitalYear) * (FracDays);

	CachedSolarFractionalYear.Value = YearInRadians;
	CachedSolarFractionalYear.Valid = true;

	return YearInRadians;
}

float UDateTimeSystemComponent::SolarDeclinationAngle(float YearInRadians)
{
	if (CachedSolarDeclinationAngle.Valid)
	{
		return CachedSolarDeclinationAngle.Value;
	}

	float A1 = 0.006918 - 
		0.399912 * FMath::Cos(YearInRadians) +
		0.070257 * FMath::Sin(YearInRadians) -
		0.006758 * FMath::Cos(2 * YearInRadians) +
		0.000907 * FMath::Sin(2 * YearInRadians) -
		0.002697 * FMath::Cos(3 * YearInRadians) +
		0.00148 * FMath::Sin(3 * YearInRadians);

	CachedSolarDeclinationAngle.Valid = true;
	CachedSolarDeclinationAngle.Value = A1;

	return A1;
}

bool UDateTimeSystemComponent::InternalDoesLeap(int Year)
{
	// Check Cache
	if (CachedDoesLeap.Valid)
	{
		return CachedDoesLeap.Value;
	}

	CachedDoesLeap.Value = DoesYearLeap(Year);
	CachedDoesLeap.Valid = true;

	return CachedDoesLeap.Value;
}

void UDateTimeSystemComponent::InternalTick(float DeltaTime)
{
	// Invalidate Caches
	Invalidate(EDateTimeSystemInvalidationTypes::Frame);

	// Increment Time
	InternalDate.Seconds += DeltaTime * TimeScale;
	auto DidRoll = SanitiseDateTime(InternalDate);

	if (DidRoll)
	{
		// Invalidate Daily Caches
		//CachedAnalyticalMonthlyHighTemp.Valid = false;
		//CachedAnalyticalMonthlyLowTemp.Valid = false;
		Invalidate(EDateTimeSystemInvalidationTypes::Day);

		// Handle rolling the starting temp of day n+1 to the ending of n
		CachedLowTemp.Value = CachedNextLowTemp.Value;
		CachedLowTemp.Valid = true;
		LastHighTemp = CachedHighTemp.Value;
		LastLowTemp = CachedLowTemp.Value;

		// Check Override
		auto Row = GetDateOverride(&InternalDate);
		if (Row)
		{
			auto asPtr = *Row;

			// We have an override.
			if (OverridedDatesSetDate)
			{
				InternalDate.SetFromRow(asPtr);
				DateOverrideCallback.Broadcast(InternalDate, asPtr->CallbackAttributes);
			}
			else
			{
				auto LV = FDateTimeSystemStruct::CreateFromRow(asPtr);
				DateOverrideCallback.Broadcast(LV, asPtr->CallbackAttributes);
			}
		}
		else
		{
			// Go ahead and compute the new values
			CachedAnalyticalMonthlyHighTemp.Value = GetAnalyticalHighForDate(InternalDate);
			CachedAnalyticalMonthlyHighTemp.Valid = true;
			CachedAnalyticalMonthlyLowTemp.Value = GetAnalyticalLowForDate(InternalDate);
			CachedAnalyticalMonthlyLowTemp.Valid = true;

			auto DummyTagContainer = FGameplayTagContainer();

			CachedNextLowTemp.Value = DailyLowModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyLowTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);
			CachedNextLowTemp.Valid = true;
			CachedHighTemp.Value = DailyHighModulation(InternalDate, DummyTagContainer, CachedAnalyticalMonthlyHighTemp.Value, CachedLowTemp.Value, CachedHighTemp.Value);
			CachedHighTemp.Valid = true;
		}

		// Broadcast that the date has changed
		DateChangeCallback.Broadcast(InternalDate);
	}
	else
	{
		// Re-validate Daily Caches. These only update daily, and no date change has occured
		CachedAnalyticalMonthlyHighTemp.Valid = true;
		CachedAnalyticalMonthlyLowTemp.Valid = true;
		CachedNextLowTemp.Valid = true;
		CachedHighTemp.Valid = true;
		CachedLowTemp.Valid = true;
	}

}

void UDateTimeSystemComponent::InternalBegin()
{
	InvLengthOfDay = 1 / LengthOfDay;

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
			if(UseDayIndexForOverride)
			{
				DateOverrides.Add(val->DayIndex, val);
			}
			else
			{
				DateOverrides.Add(GetHashForDate(val), val);
			}
		}
	}

	// Set Prevalue
	CachedLowTemp.Value = GetDailyLow(InternalDate);
	CachedLowTemp.Valid = true;
}

float UDateTimeSystemComponent::SolarTimeCorrection(float YearInRadians)
{
	if (CachedSolarTimeCorrection.Valid)
	{
		return CachedSolarTimeCorrection.Value;
	}

	float A1 = 0.000075 +
		0.001868 * FMath::Cos(YearInRadians) -
		0.032077 * FMath::Sin(YearInRadians) -
		0.014615 * FMath::Cos(YearInRadians * 2) -
		0.040849 * FMath::Sin(YearInRadians * 2);
	float EQTime = 229.18f * A1;

	CachedSolarTimeCorrection.Valid = true;
	CachedSolarTimeCorrection.Value = EQTime;

	return EQTime;
}

uint32 UDateTimeSystemComponent::GetHashForDate(FDateTimeSystemStruct* DateStruct)
{
	auto DayHash = GetTypeHash(DateStruct->Day);
	auto MonthHash = GetTypeHash(DateStruct->Month);
	auto YearHash = GetTypeHash(DateStruct->Year);
	auto Hash = HashCombine(YearHash, MonthHash);
	return HashCombine(Hash, DayHash);
}

uint32 UDateTimeSystemComponent::GetHashForDate(FDateTimeSystemDateOverrideRow* DateStruct)
{
	auto DayHash = GetTypeHash(DateStruct->Day);
	auto MonthHash = GetTypeHash(DateStruct->Month);
	auto YearHash = GetTypeHash(DateStruct->Year);
	auto Hash = HashCombine(YearHash, MonthHash);
	return HashCombine(Hash, DayHash);
}

FDateTimeSystemDateOverrideRow** UDateTimeSystemComponent::GetDateOverride(FDateTimeSystemStruct* DateStruct)
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
