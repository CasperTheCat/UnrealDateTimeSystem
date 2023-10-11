// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "Interfaces.h"

UObject *IDateTimeSystemInterface::GetDateTimeSystem_Implementation()
{
    checkNoEntry();
    return nullptr;
}

UClimateComponent *IDateTimeSystemClimateInterface::GetClimateComponent_Implementation()
{
    checkNoEntry();
    return nullptr;
}

void IDateTimeSystemCommon::GetTodaysDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
}

void IDateTimeSystemCommon::GetTodaysDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                            FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    checkNoEntry();
}

void IDateTimeSystemCommon::GetTomorrowsDate(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
}

void IDateTimeSystemCommon::GetTomorrowsDateTZ(UPARAM(ref) FDateTimeSystemStruct &DateStruct,
                                               FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    checkNoEntry();
}

void IDateTimeSystemCommon::GetYesterdaysDate(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
}

void IDateTimeSystemCommon::GetYesterdaysDateTZ(FDateTimeSystemStruct &DateStruct,
                                                UPARAM(ref) FDateTimeSystemTimezoneStruct &TimezoneInfo)
{
    checkNoEntry();
}

TTuple<float, float, float> IDateTimeSystemCommon::ComputeDeltaBetweenDatesInternal(
    UPARAM(ref) FDateTimeSystemStruct &Date1, UPARAM(ref) FDateTimeSystemStruct &Date2, FDateTimeSystemStruct &Result)
{
    checkNoEntry();
    return TTuple<float, float, float>();
}

bool IDateTimeSystemCommon::DoesYearLeap(int Year)
{
    checkNoEntry();
    return false;
}

FRotator IDateTimeSystemCommon::GetSunRotationForLocation(FVector Location)
{
    checkNoEntry();
    return FRotator();
}

FRotator IDateTimeSystemCommon::GetSunRotation()
{
    checkNoEntry();
    return FRotator();
}

FRotator IDateTimeSystemCommon::GetSunRotationForLatLong(double Latitude, double Longitude)
{
    checkNoEntry();
    return FRotator();
}

FVector IDateTimeSystemCommon::GetSunVector(float Latitude, float Longitude)
{
    checkNoEntry();
    return FVector();
}

FRotator IDateTimeSystemCommon::GetMoonRotationForLocation(FVector Location)
{
    checkNoEntry();
    return FRotator();
}

FRotator IDateTimeSystemCommon::GetMoonRotation()
{
    checkNoEntry();
    return FRotator();
}

FRotator IDateTimeSystemCommon::GetMoonRotationForLatLong(double Latitude, double Longitude)
{
    checkNoEntry();
    return FRotator();
}

FVector IDateTimeSystemCommon::GetMoonVector(float Latitude, float Longitude)
{
    checkNoEntry();
    return FVector();
}

FMatrix IDateTimeSystemCommon::GetNightSkyRotationMatrix()
{
    checkNoEntry();
    return FMatrix();
}

FMatrix IDateTimeSystemCommon::GetNightSkyRotationMatrixForLocation(FVector Location)
{
    checkNoEntry();
    return FMatrix();
}

FMatrix IDateTimeSystemCommon::GetNightSkyRotationMatrixForLatLong(double Latitude, double Longitude)
{
    checkNoEntry();
    return FMatrix();
}

FMatrix IDateTimeSystemCommon::GetLocalisedNightSkyRotationMatrix(float BaseLatitudePercent, float BaseLongitudePercent,
                                                                  FVector Location)
{
    checkNoEntry();
    return FMatrix();
}

FText IDateTimeSystemCommon::GetNameOfMonth(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return FText::FromString("Missing Month Name");
}

float IDateTimeSystemCommon::GetTimeScale()
{
    return 0.f;
}

float IDateTimeSystemCommon::GetLengthOfDay()
{
    return 0.0f;
}

void IDateTimeSystemCommon::SetUTCDateTime(FDateTimeSystemStruct &DateStruct, bool SkipInitialisation)
{
}

FDateTimeSystemStruct IDateTimeSystemCommon::GetUTCDateTime()
{
    return FDateTimeSystemStruct();
}

void IDateTimeSystemCommon::AdvanceToTime(UPARAM(ref) FDateTimeSystemStruct &DateStruct)
{
}

bool IDateTimeSystemCommon::AdvanceToClockTime(int Hour, int Minute, int Second, bool Safety)
{
    checkNoEntry();
    return false;
}

float IDateTimeSystemCommon::ComputeDeltaBetweenDates(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                      UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    checkNoEntry();
    return 0;
}

float IDateTimeSystemCommon::ComputeDeltaBetweenDatesYears(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                           UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    checkNoEntry();
    return 0;
}

float IDateTimeSystemCommon::ComputeDeltaBetweenDatesMonths(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                            UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    checkNoEntry();
    return 0;
}

float IDateTimeSystemCommon::ComputeDeltaBetweenDatesDays(UPARAM(ref) FDateTimeSystemStruct &Date1,
                                                          UPARAM(ref) FDateTimeSystemStruct &Date2)
{
    checkNoEntry();
    return 0;
}

double IDateTimeSystemCommon::ComputeDeltaBetweenDatesSeconds(UPARAM(ref) FDateTimeSystemStruct &From,
                                                              UPARAM(ref) FDateTimeSystemStruct &To)
{
    checkNoEntry();
    return 0.0;
}

void IDateTimeSystemCommon::AddDateStruct(FDateTimeSystemStruct &DateStruct)
{
}

float IDateTimeSystemCommon::GetFractionalDay(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return 0;
}

float IDateTimeSystemCommon::GetFractionalMonth(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return 0.0f;
}

float IDateTimeSystemCommon::GetFractionalOrbitalYear(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return 0;
}

float IDateTimeSystemCommon::GetFractionalCalendarYear(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return 0;
}

int IDateTimeSystemCommon::GetLengthOfCalendarYear(int Year)
{
    checkNoEntry();
    return 0;
}

int IDateTimeSystemCommon::GetDaysInCurrentMonth()
{
    checkNoEntry();
    return 0;
}

int IDateTimeSystemCommon::GetDaysInMonth(int MonthIndex)
{
    checkNoEntry();
    return 31;
}

int IDateTimeSystemCommon::GetMonthsInYear(int YearIndex)
{
    checkNoEntry();
    return 12;
}

FMatrix IDateTimeSystemCommon::RotateMatrixByNorthing(const FMatrix &RotationMatrix, FVector NorthingDirection)
{
    checkNoEntry();
    return FMatrix();
}

bool IDateTimeSystemCommon::SanitiseDateTime(FDateTimeSystemStruct &DateStruct)
{
    checkNoEntry();
    return false;
}

FRotator IDateTimeSystemCommon::GetLocalisedSunRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                        FVector Location)
{
    checkNoEntry();
    return FRotator();
}

FRotator IDateTimeSystemCommon::GetLocalisedMoonRotation(float BaseLatitudePercent, float BaseLongitudePercent,
                                                         FVector Location)
{
    checkNoEntry();
    return FRotator();
}

void IDateTimeSystemCommon::InternalTick(float DeltaTime, bool NonContiguous)
{
    checkNoEntry();
}

FVector IDateTimeSystemCommon::AlignWorldLocationInternalCoordinates(FVector WorldLocation, FVector NorthingDirection)
{
    checkNoEntry();
    return FVector();
}

FVector IDateTimeSystemCommon::RotateLocationByNorthing(FVector Location, FVector NorthingDirection)
{
    checkNoEntry();
    return FVector();
}

FRotator IDateTimeSystemCommon::RotateRotationByNorthing(FRotator Rotation, FVector NorthingDirection)
{
    checkNoEntry();
    return FRotator();
}

UDateTimeSystemCore *IDateTimeSystemCommon::GetCore()
{
    checkNoEntry();
    return nullptr;
}

bool IDateTimeSystemCommon::IsReady()
{
    checkNoEntry();
    return false;
}
